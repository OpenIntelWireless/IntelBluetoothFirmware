//
//  IntelBluetoothGen1.hpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef IntelBluetoothOpsGen1_hpp
#define IntelBluetoothOpsGen1_hpp

#include "BtIntel.h"

class IntelBluetoothOpsGen1 : public BtIntel {
    OSDeclareDefaultStructors(IntelBluetoothOpsGen1)
    
public:
    
    virtual bool setup() override;
    
    virtual bool shutdown() override;
    
    virtual bool getFirmwareName(char *fwname, size_t len) override;
    
private:
    
    bool patching(OSData *fwData, const uint8_t **fw_ptr, bool *disablePatch);
    
    bool hciReset();
    
    OSData *getFirmware(IntelVersion *ver, char *, size_t);
    
private:
    char loadedFirmwareName[64];
    
};

#endif /* IntelBluetoothOpsGen1_hpp */
