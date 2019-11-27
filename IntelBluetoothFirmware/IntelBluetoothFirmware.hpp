/* add your code here */
#ifndef IntelBluetoothFirmware_H
#define IntelBluetoothFirmware_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>

class IntelBluetoothFirmware : public IOService
{
    OSDeclareDefaultStructors (IntelBluetoothFirmware)
    
public:
    
    bool init(OSDictionary *dictionary = NULL) override;
    
    void free() override;
    
    bool start(IOService *provider) override;
    
    void stop(IOService *provider) override;
    
    IOService * probe(IOService *provider, SInt32 *score) override;
    
    IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice) override;
    
    void cleanUp();
    
    bool interruptPipeRead();
    
    void beginDownload();
    
    static void onRead(void* owner, void* parameter, IOReturn status, uint32_t bytesTransferred);
    
    void parseHCIResponse(void* response, UInt16 length, void* output, UInt8* outputLength);
    
    void onHCICommandSucceed(HciResponse *command);
    
    IOReturn getDeviceStatus(USBStatus *status);
    
    IOReturn getConfiguration(UInt8 *configNumber);
    
    bool initUSBConfiguration();
    
    bool initInterface();
    
public:
    IOUSBHostDevice* m_pDevice;
    IOUSBHostInterface* m_pInterface;
    IOUSBHostPipe* m_pInterruptPipe;
    
    IOLock* completion;
    IOUSBHostCompletion usbCompletion;
    
    IOBufferMemoryDescriptor* mReadBuffer;
};

#endif
