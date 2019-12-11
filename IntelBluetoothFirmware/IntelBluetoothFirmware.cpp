/* add your code here */
#include "BtIntel.h"

#include "IntelBluetoothFirmware.hpp"
#include <libkern/libkern.h>
#include <libkern/OSKextLib.h>
#include <libkern/version.h>
#include <libkern/OSTypes.h>
#include <IOKit/usb/StandardUSB.h>
#include "Hci.h"

#define super IOService
OSDefineMetaClassAndStructors(IntelBluetoothFirmware, IOService)

#define kReadBufferSize 4096
//com.apple.iokit.IOBluetoothHostControllerUSBTransport

enum { kMyOffPowerState = 0, kMyOnPowerState = 1 };

enum {
    kReset,
    kGetIntelVersion,
    kEnterMfg,
    kLoadFW,
    kExitMfg,
    kSetEventMask,
    kUpdateAbort,
    kUpdateDone
};

static IOPMPowerState myTwoStates[2] =
{
    { kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kIOPMPowerStateVersion1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
};

bool IntelBluetoothFirmware::init(OSDictionary *dictionary)
{
    XYLog("Driver init()\n");
    return super::init(dictionary);
}

void IntelBluetoothFirmware::free() {
    XYLog("Driver free()\n");
    super::free();
}

bool IntelBluetoothFirmware::start(IOService *provider)
{
    XYLog("Driver Start()\n");
    super::start(provider);
    m_pDevice = (IOUSBHostDevice *)provider;
    
    PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
    makeUsable();
    
    hciCommand = (HciCommandHdr *)IOMalloc(sizeof(HciCommandHdr));

    completion = IOLockAlloc();

    if (!completion) {
        return false;
    }
    
    resourceCompletion = IOLockAlloc();
    if (!resourceCompletion) {
        return false;
    }
    
    resourceCallbackCompletion = IOLockAlloc();
    if (!resourceCallbackCompletion) {
        return false;
    }

    mReadBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task
                                                              , kIODirectionIn, kReadBufferSize);
    if (!mReadBuffer) {
        XYLog("%s::fail to alloc read buffer\n", getName());
        cleanUp();
        return false;
    }

    mReadBuffer->prepare(kIODirectionIn);
    
//    return true;
    m_pDevice->setConfiguration(0);
    
    IOSleep(500);
    
    usbCompletion.owner = this;
    usbCompletion.action = onRead;
    usbCompletion.parameter = NULL;
    
    if (!m_pDevice->open(this)) {
        XYLog("start fail, can not open device\n");
        super::stop(this);
        cleanUp();
        return false;
    }

    if (!initUSBConfiguration()) {
        XYLog("init usb configuration failed\"n");
        cleanUp();
        return false;
    }
    if (!initInterface()) {
        XYLog("init usb interface failed\n");
        cleanUp();
        return false;
    }
    XYLog("usb init succeed\n");
    beginDownload();
    cleanUp();
    super::stop(this);
    return true;
}

bool IntelBluetoothFirmware::initUSBConfiguration()
{
    uint8_t configIndex = 0;
    uint8_t configNum = m_pDevice->getDeviceDescriptor()->bNumConfigurations;
    if (configNum < configIndex+configNum) {
        XYLog("config num error\n");
        return false;
    }
    const StandardUSB::ConfigurationDescriptor *configDescriptor = m_pDevice->getConfigurationDescriptor(configIndex);
    if (!configDescriptor) {
        XYLog("getConfigurationDescriptor(%d) failed\n", configIndex);
        return false;
    }
    uint8_t currentConfig = 0;
    if (getConfiguration(&currentConfig) != kIOReturnSuccess)
    {
        XYLog("get device configuration failed\n");
        return false;
    }
    if (currentConfig != 0) {
        XYLog("device configuration has been set to %d\n", currentConfig);
        return true;
    }
    IOReturn ret = m_pDevice->setConfiguration(configDescriptor->bConfigurationValue);
    if (ret != kIOReturnSuccess) {
        XYLog("set device configuration to %d failed\n", configDescriptor->bConfigurationValue);
        return false;
    }
    return true;
}

bool IntelBluetoothFirmware::initInterface()
{
    OSIterator* iterator = m_pDevice->getChildIterator(gIOServicePlane);
    OSObject* candidate = NULL;
    while(iterator != NULL && (candidate = iterator->getNextObject()) != NULL)
    {
        IOUSBHostInterface* interfaceCandidate = OSDynamicCast(IOUSBHostInterface, candidate);
        if(   interfaceCandidate != NULL
           )
        {
            XYLog("找到了只找到了！！！\n");
            m_pInterface = interfaceCandidate;
            break;
        }
    }
    OSSafeReleaseNULL(iterator);
    if ((IOService*)m_pInterface == NULL) {
        return false;
    }
    if (!m_pInterface->open(this)) {
        XYLog("can not open interface\n");
        return false;
    }
    const StandardUSB::ConfigurationDescriptor *configDescriptor = m_pInterface->getConfigurationDescriptor();
    const StandardUSB::InterfaceDescriptor *interfaceDescriptor = m_pInterface->getInterfaceDescriptor();
    if (configDescriptor == NULL || interfaceDescriptor == NULL) {
        XYLog("find descriptor NULL\n");
        return false;
    }
    const EndpointDescriptor *endpointDescriptor = NULL;
    while ((endpointDescriptor = StandardUSB::getNextEndpointDescriptor(configDescriptor, interfaceDescriptor, endpointDescriptor))) {
        uint8_t epDirection = StandardUSB::getEndpointDirection(endpointDescriptor);
        uint8_t epType = StandardUSB::getEndpointType(endpointDescriptor);
        if (epDirection == kUSBIn && epType == kUSBInterrupt) {
            XYLog("找到了找到了，找到中断端点了！！！\n");
            m_pInterruptReadPipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
            if (m_pInterruptReadPipe == NULL) {
                XYLog("copy InterruptReadPipe pipe fail\n");
                return false;
            }
            m_pInterruptReadPipe->retain();
            m_pInterruptReadPipe->release();
        } else {
            if (epDirection == kUSBOut && epType == kUSBBulk) {
                XYLog("找到了找到了，找到Bulk端点了！！！\n");
                m_pBulkWritePipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
                if (m_pBulkWritePipe == NULL) {
                    XYLog("copy Bulk pipe fail\n");
                    return false;
                }
                m_pBulkWritePipe->retain();
                m_pBulkWritePipe->release();
            }
        }
    }
    return m_pInterruptReadPipe != NULL && m_pBulkWritePipe != NULL;
}

void IntelBluetoothFirmware::cleanUp()
{
    XYLog("Clean up...\n");
    PMstop();
    if (fwData) {
        fwData->release();
        fwData = NULL;
    }
    if (mReadBuffer) {
        mReadBuffer->complete(kIODirectionIn);
        usbCompletion.owner = NULL;
        usbCompletion.action = NULL;
        OSSafeReleaseNULL(mReadBuffer);
        mReadBuffer = NULL;
    }
    if (completion) {
        IOLockFree(completion);
        completion = NULL;
    }
    if (resourceCompletion) {
        IOLockFree(resourceCompletion);
        resourceCompletion = NULL;
    }
    if (hciCommand) {
        IOFree(hciCommand, sizeof(HciCommandHdr));
        hciCommand = NULL;
    }
    if (m_pBulkWritePipe) {
        m_pBulkWritePipe->abort();
        m_pBulkWritePipe->release();
        m_pBulkWritePipe = NULL;
    }
    if (m_pInterruptReadPipe) {
        m_pInterruptReadPipe->abort();
        m_pInterruptReadPipe->release();
        m_pInterruptReadPipe = NULL;
    }
    if (m_pInterface) {
        m_pInterface->close(this);
        m_pInterface->release();
        m_pInterface = NULL;
    }
    if (m_pDevice) {
        m_pDevice->close(this);
        m_pDevice->release();
        m_pDevice = NULL;
    }
}

bool IntelBluetoothFirmware::interruptPipeRead()
{
    IOReturn result;
    isRequest = false;
    if ((result = m_pInterruptReadPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), &usbCompletion, 0)) != kIOReturnSuccess) {
        if (result == kIOUSBPipeStalled)
        {
            m_pInterruptReadPipe->clearStall(false);
        }
        return false;
    }
    return true;
}

void IntelBluetoothFirmware::beginDownload()
{
    IOLockLock(completion);
    mDeviceState = kReset;
    while (true) {
        
        if (mDeviceState == kUpdateDone || mDeviceState == kUpdateAbort) {
            break;
        }
        
        IOReturn ret;
        switch (mDeviceState) {
            case kReset:
            {
                XYLog("HCI_RESET\n");
                if ((ret = sendHCIRequest(HCI_OP_RESET, 0, NULL)) != kIOReturnSuccess) {
                    XYLog("sending initial HCI reset command failed (%d), %s\n ", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                break;
            }
            case kGetIntelVersion:
            {
                XYLog("HCI_OP_INTEL_VERSION\n");
                if ((ret = sendHCIRequest(HCI_OP_INTEL_VERSION, 0, NULL)) != kIOReturnSuccess) {
                    XYLog("Reading Intel version information failed, ret=%d\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                break;
            }
            case kEnterMfg:
            {
//                StandardUSB::DeviceRequest request =
//                {
//                    .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
//                    .bRequest = 0,
//                    .wValue = 0,
//                    .wIndex = 0,
//                    .wLength = (uint16_t)sizeof(TEST_ENTER)
//                };
//                uint32_t bt = 0;
//                m_pInterface->deviceRequest(request, (void *)TEST_ENTER, bt, 0);
//                isRequest = true;
                XYLog("HCI_OP_INTEL_ENTER_MFG\n");
                if ((ret = sendHCIRequest(HCI_OP_INTEL_ENTER_MFG, 2, (void *)ENTER_MFG_PARAM)) != kIOReturnSuccess) {
                    XYLog("Entering manufacturer mode failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                break;
            }
            case kLoadFW:
            {
                uint8_t* fw = (uint8_t*)fwData->getBytesNoCopy();
                uint8_t* fw_ptr = (uint8_t*)fw;
                while (fwData->getLength() > fw_ptr - fw) {
                    FWCommandHdr *cmd;
                    const uint8_t *cmd_param;
                    HciEventHdr *evt = NULL;
                    const uint8_t *evt_param = NULL;
                    int remain = fwData->getLength() - (fw_ptr - fw);
                   /* The first byte indicates the types of the patch command or event.
                    * 0x01 means HCI command and 0x02 is HCI event. If the first bytes
                    * in the current firmware buffer doesn't start with 0x01 or
                    * the size of remain buffer is smaller than HCI command header,
                    * the firmware file is corrupted and it should stop the patching
                    * process.
                    */
                    if (remain > HCI_COMMAND_HDR_SIZE && fw_ptr[0] != 0x01) {
                        XYLog("Intel fw corrupted: invalid cmd read");
                        mDeviceState = kUpdateAbort;
                        goto done;
                        break;
                    }
                    fw_ptr++;
                    remain--;
                    cmd = (FWCommandHdr*)(fw_ptr);
                    fw_ptr += sizeof(*cmd);
                    remain -= sizeof(*cmd);
                   /* Ensure that the remain firmware data is long enough than the length
                    * of command parameter. If not, the firmware file is corrupted.
                    */
                    if (remain < cmd->plen) {
                        XYLog("Intel fw corrupted: invalid cmd len");
                        goto done;
                        break;
                    }
                   /* If there is a command that loads a patch in the firmware
                    * file, then enable the patch upon success, otherwise just
                    * disable the manufacturer mode, for example patch activation
                    * is not required when the default firmware patch file is used
                    * because there are no patch data to load.
                    */
                    
                    cmd_param = fw_ptr;
                    fw_ptr += cmd->plen;
                    remain -= cmd->plen;
                   /* This reads the expected events when the above command is sent to the
                    * device. Some vendor commands expects more than one events, for
                    * example command status event followed by vendor specific event.
                    * For this case, it only keeps the last expected event. so the command
                    * can be sent with __hci_cmd_sync_ev() which returns the sk_buff of
                    * last expected event.
                    */
                    int evt_times = 0;
                    while (remain > HCI_EVENT_HDR_SIZE && fw_ptr[0] == 0x02) {
                        fw_ptr++;
                        remain--;
                        
                        evt_times++;

                        evt = (HciEventHdr*)(fw_ptr);
                        fw_ptr += sizeof(*evt);
                        remain -= sizeof(*evt);

                        if (remain < evt->plen) {
                            XYLog("Intel fw corrupted: invalid evt len");
                            goto done;
                            break;
                        }

                        evt_param = fw_ptr;
                        fw_ptr += evt->plen;
                        remain -= evt->plen;
                    }
                   /* Every HCI commands in the firmware file has its correspond event.
                    * If event is not found or remain is smaller than zero, the firmware
                    * file is corrupted.
                    */
                    if (!evt || !evt_param || remain < 0) {
                        XYLog("Intel fw corrupted: invalid evt read");
                        goto done;
                        break;
                    }
                    XYLog("ori=%02X\n", (uint8_t)*(uint8_t*)cmd_param);
                    XYLog("evt=%02X\n", evt->evt);
//                    bulkWrite(&hdr, cmd->plen + HCI_COMMAND_HDR_SIZE);
                    if ((ret = sendHCIRequest(USBToHost16(cmd->opcode), cmd->plen, (void *)cmd_param)) != kIOReturnSuccess) {
                        XYLog("sending Intel patch command (0x%4.4x) failed (%d) %s",
                        cmd->opcode, ret, stringFromReturn(ret));
                        goto done;
                        break;
                    }
                    for (int j=0; j<evt_times; j++) {
                        interruptPipeRead();
                        IOLockSleep(completion, this, 0);
                    }
                }
                
//                if (fwData->getLength() > HCI_EVENT_HDR_SIZE) {
//                    interruptPipeRead();
//                    IOLockSleep(completion, this, 0);
//                }
                
                mDeviceState = kExitMfg;
                break;
            }
            case kExitMfg:
            {
//                bool reset = true;
//                bool patched = true;
//                uint8_t param[] = { 0x00, 0x00 };
               /* The 2nd command parameter specifies the manufacturing exit method:
                * 0x00: Just disable the manufacturing mode (0x00).
                * 0x01: Disable manufacturing mode and reset with patches deactivated.
                * 0x02: Disable manufacturing mode and reset with patches activated.
                */
//                if (reset)
//                    param[1] |= patched ? 0x02 : 0x01;
//                StandardUSB::DeviceRequest request =
//                {
//                    .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
//                    .bRequest = 0,
//                    .wValue = 0,
//                    .wIndex = 0,
//                    .wLength = (uint16_t)sizeof(TEST_EXIT)
//                };
//                uint32_t bt = 0;
//                m_pInterface->deviceRequest(request, (void *)TEST_EXIT, bt, 0);
//                isRequest = true;
                XYLog("HCI_OP_INTEL_EXIT_MFG\n");
                if ((ret = sendHCIRequest(HCI_OP_INTEL_ENTER_MFG, 2, (void *)EXIT_MFG_PARAM)) != kIOReturnSuccess) {
                    XYLog("Exiting manufacturer mode failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                break;
            }
            case kSetEventMask:
            {
//                StandardUSB::DeviceRequest request =
//                {
//                    .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
//                    .bRequest = 0,
//                    .wValue = 0,
//                    .wIndex = 0,
//                    .wLength = (uint16_t)sizeof(TEST_EVENT)
//                };
//                uint32_t bt = 0;
//                m_pInterface->deviceRequest(request, (void *)TEST_EVENT, bt, 0);
//                isRequest = true;
                XYLog("HCI_OP_INTEL_EVENT_MASK\n");
                if ((ret = sendHCIRequest(HCI_OP_INTEL_EVENT_MASK, 8, (void *)EVENT_MASK)) != kIOReturnSuccess) {
                    XYLog("Setting Intel event mask failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                mDeviceState = kUpdateDone;
                break;
            }
            default:
                break;
        }

        if (isRequest) {
            if (!interruptPipeRead())
            {
                XYLog("hci read pipe fail. stop\n");
                break;
            }
            XYLog("interrupt wait");
            IOLockSleep(completion, this, 0);
        }
        isRequest = false;
        XYLog("interrupt continue");
    }
    
done:
    
    
    IOLockUnlock(completion);
    
    XYLog("End download\n");
    m_pInterface->close(this);
    m_pInterruptReadPipe->abort();
    m_pInterruptReadPipe->release();
    m_pInterruptReadPipe = NULL;
    m_pBulkWritePipe->abort();
    m_pBulkWritePipe->release();
    m_pBulkWritePipe = NULL;
    m_pInterface->release();
    m_pInterface = NULL;
    m_pDevice->close(this);
    m_pDevice->release();
    m_pDevice = NULL;
}

IOReturn IntelBluetoothFirmware::sendHCIRequest(uint16_t opCode, uint8_t paramLen, const void * param)
{
    isRequest = true;
    XYLog("opCode=0x%02x, paramLen=%d\n", opCode, paramLen);
    if (paramLen > 0) {
        XYLog("check=%02X\n", (uint8_t)*(uint8_t*)param);
    }
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = (uint16_t)(HCI_COMMAND_HDR_SIZE + paramLen)
    };
    uint32_t bytesTransfered;
    bzero(hciCommand, sizeof(HciCommandHdr));
    hciCommand->opcode = opCode;
    hciCommand->plen = paramLen;
    memcpy((void *)hciCommand->pData, param, paramLen);
//    BtIntel::printAllByte(&hdr, paramLen + HCI_COMMAND_HDR_SIZE);
    IOReturn ret = m_pInterface->deviceRequest(request, (void *)hciCommand, bytesTransfered, 0);
    return ret;
}

IOReturn IntelBluetoothFirmware::bulkWrite(const void *data, uint16_t length)
{
    IOMemoryDescriptor* buffer = IOMemoryDescriptor::withAddress((void*)data, length, kIODirectionOut);
    if (!buffer) {
        XYLog("Unable to allocate bulk write buffer.\n");
        return kIOReturnNoMemory;
    }
    IOReturn ret;
    if ((ret = buffer->prepare(kIODirectionOut)) != kIOReturnSuccess) {
        XYLog("Failed to prepare bulk write memory buffer, %s\n", stringFromReturn(ret));
        buffer->release();
        return ret;
    }
    if ((ret = m_pBulkWritePipe->io(buffer, buffer->getLength(), (IOUSBHostCompletion*)NULL, 0)) != kIOReturnSuccess) {
        XYLog("Failed to write to bulk pipe, %s\n", stringFromReturn(ret));
        buffer->release();
        return ret;
    }
    if ((ret = buffer->complete(kIODirectionOut)) != kIOReturnSuccess) {
        XYLog("Failed to complete bulk write memory buffer, %s\n", stringFromReturn(ret));
        buffer->release();
        return ret;
    }
    return ret;
}

void IntelBluetoothFirmware::onRead(void *owner, void *parameter, IOReturn status, uint32_t bytesTransferred)
{
    IntelBluetoothFirmware* that = (IntelBluetoothFirmware*)owner;
    IOLockLock(that->completion);
    
    switch (status) {
        case kIOReturnSuccess:
            that->parseHCIResponse(that->mReadBuffer->getBytesNoCopy(), bytesTransferred, NULL, NULL);
            break;
        case kIOReturnNotResponding:
            that->m_pInterruptReadPipe->clearStall(false);
            break;
            
        default:
            XYLog("咋了，这是咋了？\n");
            break;
    }
    
    IOLockUnlock(that->completion);
    IOLockWakeup(that->completion, that, true);
}

void IntelBluetoothFirmware::parseHCIResponse(void* response, UInt16 length, void* output, UInt8* outputLength)
{
    HciEventHdr* header = (HciEventHdr*)response;
    XYLog("recv event=0x%04x, length=%d\n", header->evt, header->plen);
    switch (header->evt) {
        case HCI_EV_CMD_COMPLETE:
            onHCICommandSucceed((HciResponse*)response, header->plen);
            break;
            
        default:
            XYLog("can not proceed response\n");
            break;
    }
}

void IntelBluetoothFirmware::onHCICommandSucceed(HciResponse *command, int length)
{
    XYLog("recv command opcode=0x%04x\n", command->opcode);
    BtIntel::printAllByte(command, length + HCI_COMMAND_HDR_SIZE);
    switch (command->opcode) {
        case HCI_OP_RESET:
            mDeviceState = kGetIntelVersion;
            break;
        case HCI_OP_INTEL_VERSION:
        {
            ver = (IntelVersion*)((uint8_t*)command + 5);
           /* fw_patch_num indicates the version of patch the device currently
            * have. If there is no patch data in the device, it is always 0x00.
            * So, if it is other than 0x00, no need to patch the device again.
            */
            if (ver->fw_patch_num) {
                XYLog("Intel device is already patched. "
                        "patch num: %02x", ver->fw_patch_num);
                mDeviceState = kSetEventMask;
                break;
            }
            char fwname[64];
            snprintf(fwname, sizeof(fwname),
            "ibt-hw-%x.%x.%x-fw-%x.%x.%x.%x.%x.bseq",
            ver->hw_platform, ver->hw_variant, ver->hw_revision,
            ver->fw_variant,  ver->fw_revision, ver->fw_build_num,
            ver->fw_build_ww, ver->fw_build_yy);
            XYLog("request firmware %s \n", fwname);
            if (!fwData) {
                fwData = requestFirmware(fwname);
                if (!fwData) {
                    XYLog("no firmware, stop\n");
                    mDeviceState = kUpdateAbort;
                    break;
                }
            }
            XYLog("request firmware success");
            mDeviceState = kEnterMfg;
            break;
        }
        case HCI_OP_INTEL_ENTER_MFG:
        {
            if (mDeviceState == kEnterMfg) {
                mDeviceState = kLoadFW;
            } else {
                mDeviceState = kSetEventMask;
            }
            break;
        }
        case HCI_OP_INTEL_EVENT_MASK:
            mDeviceState = kUpdateDone;
            break;
            
        default:
            break;
    }
}

OSData* IntelBluetoothFirmware::requestFirmware(const char* resourceName)
{
    IOLockLock(resourceCompletion);
    ResourceCallbackContext context =
    {
        .context = this,
        .resource = NULL
    };
    OSKextRequestResource(OSKextGetCurrentIdentifier(), resourceName, onLoadFW, &context, NULL);
    XYLog("request start.\n");
    IOLockSleep(resourceCompletion, this, 0);
    XYLog("是谁叫醒了我.\n");
    IOLockUnlock(resourceCompletion);
    if (context.resource == NULL) {
        XYLog("%s resource load fail.\n", resourceName);
        return NULL;
    }
    XYLog("request resource succeed.");
    return context.resource;
}

void IntelBluetoothFirmware::onLoadFW(OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context)
{
    XYLog("onLoadFW callback ret=0x%08x\n length=%d", result, resourceDataLength);
    ResourceCallbackContext *resourceContxt = (ResourceCallbackContext*)context;
    IOLockLock(resourceContxt->context->resourceCallbackCompletion);
    if (result == kOSReturnSuccess) {
        XYLog("onLoadFW return success");
        resourceContxt->resource = OSData::withBytes(resourceData, resourceDataLength);
    }
    IOLockUnlock(resourceContxt->context->resourceCallbackCompletion);
    IOLockWakeup(resourceContxt->context->resourceCompletion, resourceContxt->context, false);
    XYLog("onLoadFW wakeup");
}

IOReturn IntelBluetoothFirmware::getConfiguration(UInt8 *configNumber)
{
    uint8_t config  = 0;
    StandardUSB::DeviceRequest request;
    request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest      = kDeviceRequestGetConfiguration;
    request.wValue        = 0;
    request.wIndex        = 0;
    request.wLength       = sizeof(config);
    uint32_t bytesTransferred = 0;
    IOReturn result = m_pDevice->deviceRequest(this, request, &config, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
    *configNumber = config;
    return result;
}

IOReturn IntelBluetoothFirmware::setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice)
{
    XYLog("setPowerState powerStateOrdinal=%lu\n", powerStateOrdinal);
    return IOPMAckImplied;
}

void IntelBluetoothFirmware::stop(IOService *provider)
{
    XYLog("Driver Stop()\n");
    PMstop();
    if (fwData) {
        fwData->release();
        fwData = NULL;
    }
    if (mReadBuffer) {
        mReadBuffer->complete(kIODirectionIn);
        usbCompletion.owner = NULL;
        usbCompletion.action = NULL;
        OSSafeReleaseNULL(mReadBuffer);
        mReadBuffer = NULL;
    }
    if (completion) {
        IOLockFree(completion);
        completion = NULL;
    }
    if (resourceCompletion) {
        IOLockFree(resourceCompletion);
        resourceCompletion = NULL;
    }
    cleanUp();
    super::stop(provider);
}

IOService * IntelBluetoothFirmware::probe(IOService *provider, SInt32 *score)
{
    XYLog("Driver Probe()\n");
    if (!super::probe(provider, score)) {
        XYLog("super probe failed\n");
        return NULL;
    }
    *score = 3000;
    m_pDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!m_pDevice) {
        XYLog("%s -- %s is not usb device, class=%s\n", provider->getLocation(), provider->getName(), provider->metaClass->getClassName());
        return NULL;
    }
    UInt16 vendorID = USBToHost16(m_pDevice->getDeviceDescriptor()->idVendor);
    UInt16 productID = USBToHost16(m_pDevice->getDeviceDescriptor()->idProduct);
    XYLog("name=%s, class=%s, vendorID=0x%04X, productID=0x%04X\n", m_pDevice->getName(), provider->metaClass->getClassName(), vendorID, productID);
    m_pDevice = NULL;
    return this;
}
