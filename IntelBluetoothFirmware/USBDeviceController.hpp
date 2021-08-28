//
//  USBHCIController.hpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/5/21.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef USBDeviceController_hpp
#define USBDeviceController_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>
#include <IOKit/usb/USB.h>
#include <libkern/OSKextLib.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>

#include "Hci.h"

typedef struct {
    int status;
    uint32_t dataLen;
} InterruptResp;

class USBDeviceController : public OSObject {
    OSDeclareDefaultStructors(USBDeviceController)
    
public:
    
    virtual bool init(IOService *client, IOUSBHostDevice *dev);
    
    virtual void free() override;
    
    virtual bool initConfiguration();
    
    virtual bool findInterface();
    
    virtual bool findPipes();
    
    IOReturn bulkPipeRead(void *buf, uint32_t buf_size, uint32_t *size, uint32_t timeout);
    
    IOReturn interruptPipeRead(void *buf, uint32_t buf_size, uint32_t *size, uint32_t timeout);
    
    IOReturn sendHCIRequest(HciCommandHdr *cmd, uint32_t timeout);
    
    IOReturn bulkWrite(const void *data, uint32_t length, uint32_t timeout);
    
    const char* stringFromReturn(IOReturn code);
    
    static void interruptHandler(void *owner, void *parameter, IOReturn status, uint32_t bytesTransferred);
    
private:
    IOUSBHostDevice* m_pDevice;
    IOService*  m_pClient;
    IOUSBHostInterface* m_pInterface;
    
    IOUSBHostPipe* m_pInterruptReadPipe;
    IOUSBHostPipe* m_pBulkWritePipe;
    IOUSBHostPipe* m_pBulkReadPipe;
    
    IOLock *_hciLock;
    IOBufferMemoryDescriptor* mReadBuffer;
};

#endif /* USBDeviceController_hpp */
