//
//  IntelBTPatcher.h
//  IntelBTPatcher
//
//  Created by qcwap on 2021/2/8.
//

#ifndef IntelBTPatcher_h
#define IntelBTPatcher_h

#include <Headers/kern_patcher.hpp>

#define DRV_NAME "IntelBTPatcher"

class BluetoothDeviceAddress;

class CIntelBTPatcher {
public:
    bool init();
    void free();
    
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);
    static IOReturn newFindQueueRequest(void *that, unsigned short arg1, void *addr, unsigned short arg2, bool arg3, void **hciRequestPtr);
    
    mach_vm_address_t oldFindQueueRequest {};
};

#endif /* IntelBTPatcher_h */
