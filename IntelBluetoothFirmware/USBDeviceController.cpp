//
//  USBHCIController.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/5/21.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "USBDeviceController.hpp"
#include "Log.h"
#include "Hci.h"

#define super OSObject
OSDefineMetaClassAndStructors(USBDeviceController, OSObject)

#define kReadBufferSize 4096

bool USBDeviceController::
init(IOService *client, IOUSBHostDevice *dev)
{
    XYLog("%s\n", __PRETTY_FUNCTION__);
    if (!super::init()) {
        return false;
    }
    _hciLock = IOLockAlloc();
    if (!_hciLock) {
        return false;
    }
    mReadBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task
                                                              , kIODirectionIn, kReadBufferSize);
    if (!mReadBuffer) {
        XYLog("Fail to alloc read buffer\n");
        return false;
    }
    
    mReadBuffer->prepare(kIODirectionIn);
    m_pDevice = dev;
    m_pClient = client;
    return true;
}

bool USBDeviceController::
findInterface()
{
    XYLog("%s\n", __PRETTY_FUNCTION__);
    OSIterator* iterator = m_pDevice->getChildIterator(gIOServicePlane);
    OSObject* candidate = NULL;
    if (iterator == NULL) {
        XYLog("can not create child iterator\n");
        return false;
    }
    while((candidate = iterator->getNextObject()) != NULL) {
        IOUSBHostInterface* interfaceCandidate = OSDynamicCast(IOUSBHostInterface, candidate);
        if(interfaceCandidate != NULL) {
            XYLog("Found interface!!!\n");
            m_pInterface = interfaceCandidate;
            break;
        }
    }
    OSSafeReleaseNULL(iterator);
    if (m_pInterface == NULL) {
        return false;
    }
    if (!m_pInterface->open(m_pClient)) {
        XYLog("can not open interface\n");
        return false;
    }
    return true;
}

void USBDeviceController::
free()
{
    XYLog("%s\n", __PRETTY_FUNCTION__);
    if (m_pBulkWritePipe) {
        m_pBulkWritePipe->abort();
        OSSafeReleaseNULL(m_pBulkWritePipe);
    }
    if (m_pBulkReadPipe) {
        m_pBulkReadPipe->abort();
        OSSafeReleaseNULL(m_pBulkReadPipe);
    }
    if (m_pInterruptReadPipe) {
        m_pInterruptReadPipe->abort();
        OSSafeReleaseNULL(m_pInterruptReadPipe);
    }
    if (mReadBuffer) {
        mReadBuffer->complete(kIODirectionIn);
        OSSafeReleaseNULL(mReadBuffer);
    }
    if (_hciLock) {
        IOLockFree(_hciLock);
        _hciLock = NULL;
    }
    if (m_pInterface) {
        if (m_pClient && m_pInterface->isOpen(m_pClient)) {
            m_pInterface->close(m_pClient);
            m_pClient = NULL;
        }
        m_pInterface = NULL;
    }
    m_pDevice = NULL;
    super::free();
}

bool USBDeviceController::
initConfiguration()
{
    XYLog("%s\n", __PRETTY_FUNCTION__);
    uint8_t configIndex = 0;
    uint8_t configNum = m_pDevice->getDeviceDescriptor()->bNumConfigurations;
    if (configNum < configIndex + configNum) {
        XYLog("config num error (num: %d)\n", configNum);
        return false;
    }
    const StandardUSB::ConfigurationDescriptor *configDescriptor = m_pDevice->getConfigurationDescriptor(configIndex);
    if (!configDescriptor) {
        XYLog("getConfigurationDescriptor(%d) failed\n", configIndex);
        return false;
    }
    XYLog("set configuration to %d\n", configDescriptor->bConfigurationValue);
    IOReturn ret = m_pDevice->setConfiguration(configDescriptor->bConfigurationValue);
    if (ret != kIOReturnSuccess) {
        XYLog("set device configuration to %d failed\n", configDescriptor->bConfigurationValue);
        return false;
    }
    return true;
}

bool USBDeviceController::
findPipes()
{
    XYLog("%s\n", __PRETTY_FUNCTION__);
    const StandardUSB::ConfigurationDescriptor *configDescriptor;
    const StandardUSB::InterfaceDescriptor *interfaceDescriptor;
    const EndpointDescriptor *endpointDescriptor;
    uint8_t epDirection, epType;
    
    configDescriptor = m_pInterface->getConfigurationDescriptor();
    interfaceDescriptor = m_pInterface->getInterfaceDescriptor();
    if (configDescriptor == NULL || interfaceDescriptor == NULL) {
        XYLog("Find descriptor NULL\n");
        return false;
    }
    endpointDescriptor = NULL;
    while ((endpointDescriptor = StandardUSB::getNextEndpointDescriptor(configDescriptor, interfaceDescriptor, endpointDescriptor))) {
        epDirection = StandardUSB::getEndpointDirection(endpointDescriptor);
        epType = StandardUSB::getEndpointType(endpointDescriptor);
        if (epDirection == kUSBIn && epType == kUSBInterrupt) {
            XYLog("Found Interrupt endpoint!\n");
            m_pInterruptReadPipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
            if (m_pInterruptReadPipe == NULL) {
                XYLog("Copy InterruptReadPipe pipe fail\n");
                return false;
            }
            m_pInterruptReadPipe->retain();
            m_pInterruptReadPipe->release();
        } else {
            if (epDirection == kUSBOut && epType == kUSBBulk) {
                XYLog("Found Bulk out endpoint!\n");
                m_pBulkWritePipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
                if (m_pBulkWritePipe == NULL) {
                    XYLog("Copy Bulk pipe fail\n");
                    return false;
                }
                m_pBulkWritePipe->retain();
                m_pBulkWritePipe->release();
            } else {
                if (epDirection == kUSBIn && epType == kUSBBulk) {
                    XYLog("Found Bulk in endpoint!\n");
                    m_pBulkReadPipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
                    if (m_pBulkReadPipe == NULL) {
                        XYLog("Copy Bulk pipe fail\n");
                        return false;
                    }
                    m_pBulkReadPipe->retain();
                    m_pBulkReadPipe->release();
                }
            }
        }
    }
    return (m_pInterruptReadPipe != NULL && m_pBulkWritePipe != NULL && m_pBulkReadPipe != NULL);
}

IOReturn USBDeviceController::
bulkPipeRead(void *buf, uint32_t buf_size, uint32_t *size, uint32_t timeout)
{
    uint32_t actualLength = 0;
    IOReturn ret = m_pBulkReadPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), actualLength, timeout);
    if (ret == kIOUSBPipeStalled) {
        m_pBulkReadPipe->clearStall(true);
        ret = m_pBulkReadPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), actualLength, timeout);
    }
    if (ret == kIOReturnSuccess) {
        if (buf && actualLength > buf_size) {
            XYLog("%s buf size too small. buflen: %d act: %d\n", __FUNCTION__, buf_size, actualLength);
        }
        if (buf) {
            memcpy(buf, mReadBuffer->getBytesNoCopy(), min(actualLength, buf_size));
        }
        if (size) {
            *size = min(actualLength, buf_size);
        }
    } else {
        XYLog("%s failed: %s %d\n", __FUNCTION__, stringFromReturn(ret), ret);
    }
    return ret;
}

void USBDeviceController::
interruptHandler(void *owner, void *parameter, IOReturn status, uint32_t bytesTransferred)
{
    USBDeviceController *controller = OSDynamicCast(USBDeviceController, (OSObject *)owner);
    if (!controller || !parameter) {
        return;
    }
    switch (status) {
        case kIOReturnSuccess:
            break;
        case kIOReturnNotResponding:
            controller->m_pInterruptReadPipe->clearStall(false);
        default:
            XYLog("%s status: %s (%d) len: %d\n", __FUNCTION__, controller->stringFromReturn(status), status, bytesTransferred);
            break;
    }
    
    InterruptResp *resp = (InterruptResp *)parameter;
    resp->status = status;
    resp->dataLen = bytesTransferred;
    IOLockWakeup(controller->_hciLock, controller, true);
}

IOReturn USBDeviceController::
interruptPipeRead(void *buf, uint32_t buf_size, uint32_t *size, uint32_t timeout)
{
    AbsoluteTime deadline;
    IOUSBHostCompletion comple;
    InterruptResp interrupResp;
    
    clock_interval_to_deadline(timeout, kMillisecondScale, reinterpret_cast<uint64_t*> (&deadline));
    memset(&interrupResp, 0, sizeof(interrupResp));
    comple.action = interruptHandler;
    comple.owner = this;
    comple.parameter = &interrupResp;
    
    IOReturn ret = m_pInterruptReadPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), &comple, 0);
    if (ret == kIOUSBPipeStalled) {
        m_pInterruptReadPipe->clearStall(true);
        ret = m_pInterruptReadPipe->io(mReadBuffer, (uint32_t)mReadBuffer->getLength(), &comple, 0);
    }
    
    if (ret == kIOReturnSuccess) {
        IOLockLock(_hciLock);
        if (IOLockSleepDeadline(_hciLock, this, deadline, THREAD_INTERRUPTIBLE) != THREAD_AWAKENED) {
            IOLockUnlock(_hciLock);
            m_pInterruptReadPipe->abort();
            XYLog("%s Timeout\n", __FUNCTION__);
            return kIOReturnTimeout;
        }
        if (interrupResp.dataLen <= 0) {
            IOLockUnlock(_hciLock);
            XYLog("%s invalid response size: %d\n", __FUNCTION__, interrupResp.dataLen);
            return kIOReturnError;
        }
        if (buf && interrupResp.dataLen > buf_size) {
            XYLog("%s buf size too small. buflen: %d act: %d\n", __FUNCTION__, buf_size, interrupResp.dataLen);
        }
        if (buf) {
            memcpy(buf, mReadBuffer->getBytesNoCopy(), min(interrupResp.dataLen, buf_size));
        }
        if (size) {
            *size = min(interrupResp.dataLen, buf_size);
        }
        IOLockUnlock(_hciLock);
    } else {
        XYLog("%s failed: %s %d\n", __FUNCTION__, stringFromReturn(ret), ret);
    }
    return ret;
}

IOReturn USBDeviceController::
sendHCIRequest(HciCommandHdr *cmd, uint32_t timeout)
{
    uint32_t actualLength;
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = (uint16_t)(HCI_COMMAND_HDR_SIZE + cmd->len)
    };
    
    return m_pInterface->deviceRequest(request, cmd, actualLength, timeout);
}

IOReturn USBDeviceController::
bulkWrite(const void *data, uint32_t length, uint32_t timeout)
{
    IOMemoryDescriptor* buffer = IOMemoryDescriptor::withAddress((void *)data, length, kIODirectionOut);
    if (!buffer) {
        XYLog("Unable to allocate bulk write buffer.\n");
        return kIOReturnNoMemory;
    }
    IOReturn ret;
    uint32_t actLen = 0;
    if ((ret = buffer->prepare(kIODirectionOut)) != kIOReturnSuccess) {
        XYLog("Failed to prepare bulk write memory buffer (error %d).\n", ret);
        buffer->release();
        return ret;
    }
    if ((ret = m_pBulkWritePipe->io(buffer, (uint32_t)buffer->getLength(), actLen, timeout)) != kIOReturnSuccess) {
        XYLog("Failed to write to bulk pipe (error %d)\n", ret);
        buffer->complete();
        buffer->release();
        return ret;
    }
    if ((ret = buffer->complete(kIODirectionOut)) != kIOReturnSuccess) {
        XYLog("Failed to complete bulk write memory buffer (error %d)\n", ret);
        buffer->release();
        return ret;
    }
    return ret;
}

const char* USBDeviceController::
stringFromReturn(IOReturn code)
{
    return m_pDevice->stringFromReturn(code);
}
