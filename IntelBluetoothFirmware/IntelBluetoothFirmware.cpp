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
    
    mReadBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task
                                                              , kIODirectionIn, kReadBufferSize);
    if (!mReadBuffer) {
        IOLog("%s::fail to alloc read buffer\n", getName());
        OSSafeReleaseNULL(mReadBuffer);
        return false;
    }
    
    mReadBuffer->prepare(kIODirectionIn);
    
//    m_pDevice->reset();
//    return false;
    m_pDevice->setConfiguration(0);
    
    IOSleep(300);
    
    usbCompletion.owner = this;
    usbCompletion.action = onRead;
    usbCompletion.parameter = NULL;
    
    if (!m_pDevice->open(this)) {
        IOLog("start fail, can not open device\n");
        return false;
    }

    if (!initUSBConfiguration()) {
        IOLog("init usb configuration failed\"n");
        return false;
    }
    
//    StandardUSB::DeviceRequest deviceRequest =
//    {
//        .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
//        .bRequest = 0,
//        .wValue = 0,
//        .wIndex = 0,
//        .wLength = 2
//    };
//    IntelVersion recv;
//    bzero(&recv, sizeof(recv));
//    uint32_t bts = 0;
//    int opcode = HCI_RESET;
//    if (m_pDevice->deviceRequest(this, deviceRequest, (void *)&opcode, bts, kUSBHostClassRequestCompletionTimeout))
//    {
//        IOLog("get version fail\n");
//        return false;
//    }
//    IOLog("get length %d, %d-%d-%d\n", bts, recv.hw_revision, recv.hw_platform, recv.status);
    if (!initInterface()) {
        IOLog("init usb interface failed\n");
        return false;
    }
    IOLog("usb init succeed\n");
    beginDownload();
    return true;
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
    
    while (true) {

        //TEST TODO
        
        StandardUSB::DeviceRequest request =
        {
            .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
            .bRequest = 0,
            .wValue = 0,
            .wIndex = 0,
            .wLength = 2
        };
        uint32_t bytesTransfered;
        m_pInterface->deviceRequest(request, (void *)NULL, bytesTransfered, 0);

        if (!interruptPipeRead()) {
            IOLog("hci read pipe fail. stop\n");
            break;
        }
        IOLockSleep(completion, this, 0);
        //TODO
        break;
    }
    
    
    IOLockUnlock(completion);
    
    cleanUp();
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
    switch (header->evt) {
        case HCI_EV_CMD_COMPLETE:
            onHCICommandSucceed((HciResponse*)response);
            break;
            
        default:
            IOLog("recv event=0x%04x, length=%d, but can not proceed\n", header->evt, header->plen);
            break;
    }
}

void IntelBluetoothFirmware::onHCICommandSucceed(HciResponse *command)
{
    IOLog("recv command opcode=0x%04x\n", command->opcode);
    IntelVersion* ver = (IntelVersion*)((uint8_t*)command + 6);
    BtIntel::printIntelVersion(ver);
}



IOReturn IntelBluetoothFirmware::getDeviceStatus(USBStatus *status)
{
    uint16_t stat       = 0;
    StandardUSB::DeviceRequest request;
    request.bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionIn, kRequestTypeStandard, kRequestRecipientDevice);
    request.bRequest      = kDeviceRequestGetStatus;
    request.wValue        = 0;
    request.wIndex        = 0;
    request.wLength       = sizeof(stat);
    uint32_t bytesTransferred = 0;
    IOReturn result = m_pDevice->deviceRequest(this, request, &stat, bytesTransferred, kUSBHostStandardRequestCompletionTimeout);
    *status = stat;
    return result;
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
    super::stop(provider);
    PMstop();
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
    cleanUp();
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
