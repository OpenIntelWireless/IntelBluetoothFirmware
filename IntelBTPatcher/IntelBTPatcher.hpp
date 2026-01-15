//
//  IntelBTPatcher.h
//  IntelBTPatcher
//
//  Created by zxystd <zxystd@foxmail.com> on 2021/2/8.
//

#ifndef IntelBTPatcher_h
#define IntelBTPatcher_h

#include <Headers/kern_patcher.hpp>

#include <IOKit/usb/IOUSBHostDevice.h>

#define DRV_NAME "ibtp"

class BluetoothDeviceAddress;

typedef struct {
    void *owner;
    IOMemoryDescriptor *dataBuffer;
    IOUSBHostCompletionAction action;
} AsyncOwnerData;

typedef struct __attribute__((packed))
{
    uint16_t    opcode;    /* OCF & OGF */
    uint8_t     len;
    uint8_t     data[];
} HciCommandHdr;

typedef struct __attribute__((packed))
{
    uint8_t     evt;
    uint8_t     len;
    uint8_t     data[];
} HciEventHdr;

const char *requestDirectionNames[] = {
    "OUT",
    "IN"
};

const char *requestTypeNames[] = {
    "Standard",
    "Class",
    "Vendor"
};

const char *requestRecipientNames[] = {
    "Device",
    "Interface",
    "Endpoint",
    "Other"
};

const char* _hexDumpHCIData(uint8_t *buf, size_t len)
{
    ssize_t str_len = len * 3 + 1;
    char *str = (char*)IOMalloc(str_len);
    if (!str)
        return nullptr;
    for (size_t i = 0; i < len; i++)
    snprintf(str + 3 * i, (len - i) * 3, "%02x ", buf[i]);
    str[MAX(str_len - 2, 0)] = 0;
    return str;
}

class CIntelBTPatcher {
public:
    bool init();
    void free();
    
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);
    static IOReturn newFindQueueRequest(void *that, unsigned short arg1, void *addr, unsigned short arg2, bool arg3, void **hciRequestPtr);
    
    static IOReturn newHostDeviceRequest(void *that, IOService *provider, StandardUSB::DeviceRequest &request, void *data, IOMemoryDescriptor *descriptor, unsigned int &length,IOUSBHostCompletion *completion, unsigned int timeout);
    static IOReturn newAsyncIO(void *that, IOMemoryDescriptor* dataBuffer, uint32_t dataBufferLength, IOUSBHostCompletion* completion, uint32_t completionTimeoutMs);
    static int newInitPipe(void *that, StandardUSB::EndpointDescriptor const *descriptor, StandardUSB::SuperSpeedEndpointCompanionDescriptor const *superDescriptor,AppleUSBHostController *controller, IOUSBHostDevice *device, IOUSBHostInterface *interface, unsigned char, unsigned short);

    
    mach_vm_address_t oldFindQueueRequest {};
    mach_vm_address_t oldHostDeviceRequest {};
    mach_vm_address_t oldAsyncIO {};
    mach_vm_address_t oldInitPipe {};
    
private:
    static void *_hookPipeInstance;
    static AsyncOwnerData *_interruptPipeAsyncOwner;
    static bool _randomAddressInit;
};

#endif /* IntelBTPatcher_h */
