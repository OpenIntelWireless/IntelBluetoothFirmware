//
//  IntelBluetoothOpsGen2.hpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef IntelBluetoothOpsGen2_hpp
#define IntelBluetoothOpsGen2_hpp

#include "BtIntel.h"

#define CMD_WRITE_BOOT_PARAMS    0xfc0e
struct cmd_write_boot_params {
    uint32_t boot_addr;
    uint8_t  fw_build_num;
    uint8_t  fw_build_ww;
    uint8_t  fw_build_yy;
} __attribute__((packed));

class IntelBluetoothOpsGen2 : public BtIntel {
    OSDeclareDefaultStructors(IntelBluetoothOpsGen2)
    
public:
    
    virtual bool setup() override;
    
    virtual bool shutdown() override;
    
    virtual bool getFirmwareName(char *fwname, size_t len) override;
    
protected:
    
    bool downloadFirmwarePayload(OSData *fwData, size_t offset);
    
    bool rsaHeaderSecureSend(OSData *fwData);
    
    bool firmwareVersion(uint8_t num, uint8_t ww, uint8_t yy, OSData *fwData, uint32_t *bootAddr);
    
private:
    
    bool getFirmware(IntelVersion *ver, IntelBootParams *params, char *name, size_t len, const char *suffix);
    
    IOReturn downloadFirmwareData(IntelVersion *ver, OSData *fwData, uint32_t *bootParams);
    
    bool downloadFirmware(IntelVersion *ver, IntelBootParams *params, uint32_t *bootParams);
    
private:
    char loadedFirmwareName[64];
    
};

#endif /* IntelBluetoothOpsGen2_hpp */
