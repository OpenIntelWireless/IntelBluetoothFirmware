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
    IOLog("Driver init()\n");
    return super::init(dictionary);
}

void IntelBluetoothFirmware::free() {
    IOLog("Driver free()\n");
    super::free();
}

bool IntelBluetoothFirmware::start(IOService *provider)
{
    IOLog("Driver Start()\n");
    if (!super::start(provider))
    {
        IOLog("super start failed. stop!\n");
        return false;
    }
    
    PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
    makeUsable();

    completion = IOLockAlloc();

    if (!completion) {
        return false;
    }
    
    resourceCompletion = IOLockAlloc();
    if (!resourceCompletion) {
        return false;
    }

    mReadBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task
                                                              , kIODirectionIn, kReadBufferSize);
    if (!mReadBuffer) {
        IOLog("%s::fail to alloc read buffer\n", getName());
        OSSafeReleaseNULL(mReadBuffer);
        return false;
    }

    mReadBuffer->prepare(kIODirectionIn);
    
//    m_pDevice->reset();
//    return true;
    m_pDevice->setConfiguration(0);
    
    IOSleep(300);
    
    usbCompletion.owner = this;
    usbCompletion.action = onRead;
    usbCompletion.parameter = NULL;
    
    if (!m_pDevice->open(this)) {
        IOLog("start fail, can not open device\n");
        cleanUp();
        return false;
    }

    if (!initUSBConfiguration()) {
        IOLog("init usb configuration failed\"n");
        cleanUp();
        return false;
    }
    if (!initInterface()) {
        IOLog("init usb interface failed\n");
        cleanUp();
        return false;
    }
    IOLog("usb init succeed\n");
    beginDownload();
    PMstop();
    cleanUp();
    return false;
}

bool IntelBluetoothFirmware::initUSBConfiguration()
{
    uint8_t configIndex = 0;
    uint8_t configNum = m_pDevice->getDeviceDescriptor()->bNumConfigurations;
    if (configNum < configIndex+configNum) {
        IOLog("config num error\n");
        return false;
    }
    const StandardUSB::ConfigurationDescriptor *configDescriptor = m_pDevice->getConfigurationDescriptor(configIndex);
    if (!configDescriptor) {
        IOLog("getConfigurationDescriptor(%d) failed\n", configIndex);
        return false;
    }
    uint8_t currentConfig = 0;
    if (getConfiguration(&currentConfig) != kIOReturnSuccess)
    {
        IOLog("get device configuration failed\n");
        return false;
    }
    if (currentConfig != 0) {
        IOLog("device configuration has been set to %d\n", currentConfig);
        return true;
    }
    IOReturn ret = m_pDevice->setConfiguration(configDescriptor->bConfigurationValue);
    if (ret != kIOReturnSuccess) {
        IOLog("set device configuration to %d failed\n", configDescriptor->bConfigurationValue);
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
            IOLog("找到了只找到了！！！\n");
            m_pInterface = interfaceCandidate;
            break;
        }
    }
    OSSafeReleaseNULL(iterator);
    if ((IOService*)m_pInterface == NULL) {
        return false;
    }
    if (!m_pInterface->open(this)) {
        IOLog("can not open interface\n");
        return false;
    }
    const StandardUSB::ConfigurationDescriptor *configDescriptor = m_pInterface->getConfigurationDescriptor();
    const StandardUSB::InterfaceDescriptor *interfaceDescriptor = m_pInterface->getInterfaceDescriptor();
    if (configDescriptor == NULL || interfaceDescriptor == NULL) {
        IOLog("find descriptor NULL\n");
        return false;
    }
    const EndpointDescriptor *endpointDescriptor = NULL;
    while ((endpointDescriptor = StandardUSB::getNextEndpointDescriptor(configDescriptor, interfaceDescriptor, endpointDescriptor))) {
        uint8_t epDirection = StandardUSB::getEndpointDirection(endpointDescriptor);
        uint8_t epType = StandardUSB::getEndpointType(endpointDescriptor);
        if (epDirection == kUSBIn && epType == kUSBInterrupt) {
            IOLog("找到了找到了，找到端点了！！！\n");
            m_pInterruptPipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
            if (m_pInterruptPipe == NULL) {
                IOLog("copy pipe fail\n");
                return false;
            }
            m_pInterruptPipe->retain();
            m_pInterruptPipe->release();
            return (IOService*)m_pInterface != NULL;
        }
    }
    return false;
}

void IntelBluetoothFirmware::cleanUp()
{
    IOLog("Clean up...\n");
    if (m_pInterruptPipe) {
        m_pInterruptPipe->abort();
        m_pInterruptPipe->release();
        m_pInterruptPipe = NULL;
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
    if ((result = m_pInterruptPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), &usbCompletion, 0)) != kIOReturnSuccess) {
        if (result == kIOUSBPipeStalled)
        {
            m_pInterruptPipe->clearStall(false);
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
                IOLog("HCI_RESET\n");
                if ((ret = sendHCIRequest(HCI_OP_RESET, 0, NULL)) != kIOReturnSuccess) {
                    IOLog("sending initial HCI reset command failed (%d), %s\n ", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                mDeviceState = kGetIntelVersion;
                break;
            }
            case kGetIntelVersion:
            {
                IOLog("HCI_OP_INTEL_VERSION\n");
                if ((ret = sendHCIRequest(HCI_OP_INTEL_VERSION, 0, NULL)) != kIOReturnSuccess) {
                    IOLog("Reading Intel version information failed, ret=%d\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                break;
            }
            case kEnterMfg:
            {
                uint8_t param[2] = { 0x01, 0x00 };
                if ((ret = sendHCIRequest(HCI_OP_INTEL_ENTER_MFG, 2, (void *)param)) != kIOReturnSuccess) {
                    IOLog("Entering manufacturer mode failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                mDeviceState = kExitMfg;
                break;
            }
            case kLoadFW:
                
                
                mDeviceState = kExitMfg;
                break;
            case kExitMfg:
            {
                uint8_t param[] = { 0x00, 0x00 };
                bool reset = true;
                bool patched = true;
               /* The 2nd command parameter specifies the manufacturing exit method:
                * 0x00: Just disable the manufacturing mode (0x00).
                * 0x01: Disable manufacturing mode and reset with patches deactivated.
                * 0x02: Disable manufacturing mode and reset with patches activated.
                */
                if (reset)
                    param[1] |= patched ? 0x02 : 0x01;
                if ((ret = sendHCIRequest(HCI_OP_INTEL_ENTER_MFG, 2, (void *)param)) != kIOReturnSuccess) {
                    IOLog("Exiting manufacturer mode failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                mDeviceState = kSetEventMask;
                break;
            }
            case kSetEventMask:
            {
                uint8_t mask[8] = { 0x87, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                if ((ret = sendHCIRequest(HCI_OP_INTEL_EVENT_MASK, 8, (void *)mask)) != kIOReturnSuccess) {
                    IOLog("Setting Intel event mask failed (%d)\n %s", ret, stringFromReturn(ret));
                    goto done;
                    break;
                }
                mDeviceState = kUpdateDone;
                break;
            }
            default:
                break;
        }

        if (!interruptPipeRead()) {
            IOLog("hci read pipe fail. stop\n");
            break;
        }
        
        
        IOLockSleep(completion, this, 0);
    }
    
done:
    
    
    IOLockUnlock(completion);
    
    IOLog("End download\n");
    m_pInterface->close(this);
    m_pInterruptPipe->abort();
    m_pInterruptPipe->release();
    m_pInterruptPipe = NULL;
    m_pInterface->release();
    m_pInterface = NULL;
    m_pDevice->close(this);
    m_pDevice->release();
    m_pDevice  = NULL;
}

IOReturn IntelBluetoothFirmware::sendHCIRequest(uint16_t opCode, uint8_t paramLen, void * param)
{
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = (uint16_t)(2 + paramLen)
    };
    uint32_t bytesTransfered;
    HciCommandHdr hdr;
    bzero(&hdr, sizeof(HciCommandHdr));
    hdr.opcode = opCode;
    hdr.plen = paramLen;
    hdr.pData = param;
    BtIntel::printAllByte(&hdr, paramLen + 2);
    return m_pInterface->deviceRequest(request, (void *)&hdr, bytesTransfered, 0);
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
            that->m_pInterruptPipe->clearStall(false);
            break;
            
        default:
            break;
    }
    
    IOLockUnlock(that->completion);
    IOLockWakeup(that->completion, that, true);
}

void IntelBluetoothFirmware::parseHCIResponse(void* response, UInt16 length, void* output, UInt8* outputLength)
{
    HciEventHdr* header = (HciEventHdr*)response;
    IOLog("recv event=0x%04x, length=%d\n", header->evt, header->plen);
    switch (header->evt) {
        case HCI_EV_CMD_COMPLETE:
            onHCICommandSucceed((HciResponse*)response, header->plen);
            break;
            
        default:
            IOLog("can not proceed response\n");
            break;
    }
}

void IntelBluetoothFirmware::onHCICommandSucceed(HciResponse *command, int length)
{
    IOLog("recv command opcode=0x%04x\n", command->opcode);
    BtIntel::printAllByte(command, length);
    switch (command->opcode) {
        case HCI_OP_INTEL_RESET:
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
                IOLog("Intel device is already patched. "
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
            IOLog("request firmware %s \n", fwname);
            if (!fwData) {
                fwData = requestFirmware(fwname);
                if (!fwData) {
                    IOLog("no firmware, stop\n");
                    mDeviceState = kUpdateAbort;
                    break;
                }
            }
            mDeviceState = kUpdateDone;
            break;
        }
        case HCI_OP_INTEL_ENTER_MFG:
        {
            break;
        }
            
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
    IOLog("request start.\n");
    IOLockSleep(resourceCompletion, this, 0);
    IOLockUnlock(resourceCompletion);
    if (context.resource == NULL) {
        IOLog("%s resource load fail.\n", resourceName);
        return NULL;
    }
    IOLog("request resource succeed.");
    return context.resource;
}

void IntelBluetoothFirmware::onLoadFW(OSKextRequestTag requestTag, OSReturn result, const void *resourceData, uint32_t resourceDataLength, void *context)
{
    IOLog("onLoadFW callback ret=%d\n length=%d", result, resourceDataLength);
    ResourceCallbackContext *resourceContxt = (ResourceCallbackContext*)context;
    IOLockLock(resourceContxt->context->resourceCompletion);
    if (result == kOSReturnSuccess) {
        IOLog("onLoadFW return success");
        resourceContxt->resource = OSData::withBytes(resourceData, resourceDataLength);
    }
    IOLockUnlock(resourceContxt->context->resourceCompletion);
    IOLockWakeup(resourceContxt->context->resourceCompletion, resourceContxt->context, true);
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
    IOLog("setPowerState powerStateOrdinal=%lu\n", powerStateOrdinal);
    return IOPMAckImplied;
}

void IntelBluetoothFirmware::stop(IOService *provider)
{
    IOLog("Driver Stop()\n");
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
    IOLog("Driver Probe()\n");
    if (!super::probe(provider, score)) {
        IOLog("super probe failed\n");
        return NULL;
    }
    //*score = 3000;
    m_pDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!m_pDevice) {
        IOLog("%s -- %s is not usb device, class=%s\n", provider->getLocation(), provider->getName(), provider->metaClass->getClassName());
        return NULL;
    }
    m_pDevice->retain();
    UInt16 vendorID = USBToHost16(m_pDevice->getDeviceDescriptor()->idVendor);
    UInt16 productID = USBToHost16(m_pDevice->getDeviceDescriptor()->idProduct);
    IOLog("name=%s, class=%s, vendorID=0x%04X, productID=0x%04X\n", m_pDevice->getName(), provider->metaClass->getClassName(), vendorID, productID);
    return this;
}
