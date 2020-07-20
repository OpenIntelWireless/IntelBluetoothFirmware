/* add your code here */
#ifndef IntelBluetoothFirmware_H
#define IntelBluetoothFirmware_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>
#include <IOKit/usb/USB.h>
#include <libkern/OSKextLib.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
#include "Log.h"
#include "FwData.h"

enum BTType {
    kTypeOld,
    kTypeNew,
} ;

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
    
    bool bulkPipeRead();
    
    void beginDownload();
    
    void beginDownloadNew();
    
    IOReturn bulkWrite(const void *data, uint16_t length);
    
    static void onRead(void* owner, void* parameter, IOReturn status, uint32_t bytesTransferred);
    
    IOReturn sendHCIRequest(uint16_t opCode, uint8_t paramLen, const void * param);
    
    int securedSend(uint8_t fragmentType, uint32_t plen, const uint8_t *p);
    
    void parseHCIResponse(void* response, UInt16 length, void* output, UInt8* outputLength);
    
    void onHCICommandSucceed(HciResponse *command, int length);
    
    void onHCICommandSucceedNew(HciResponse *command, int length);
    
    bool initUSBConfiguration();
    
    bool initInterface();
    
    void publishReg(bool isSucceed);
    
public:
    
    
    IOUSBHostDevice* m_pDevice;
    IOUSBHostInterface* m_pInterface;
    IOUSBHostPipe* m_pInterruptReadPipe;
    IOUSBHostPipe* m_pBulkWritePipe;
    IOUSBHostPipe* m_pBulkReadPipe;
    
    IOLock* completion;
    IOUSBHostCompletion usbCompletion;
    
    int mDeviceState;
    IntelVersion *ver;
    IntelBootParams *params;
    
    IOBufferMemoryDescriptor* mReadBuffer;
    
private:
    bool isRequest;
    OSData *fwData;
    char firmwareName[64];
    HciCommandHdr *hciCommand;
    struct ResourceCallbackContext
    {
        IntelBluetoothFirmware* context;
        OSData* resource;
    };
    BTType currentType;
    uint32_t boot_param;
};

#endif
