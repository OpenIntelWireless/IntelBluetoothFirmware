//
//  BtIntel.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef BtIntel_h
#define BtIntel_h

#include <libkern/libkern.h>
#include "Hci.h"

typedef struct __attribute__((packed)) {
    uint8_t status;
    uint8_t hw_platform;
    uint8_t hw_variant;
    uint8_t hw_revision;
    uint8_t fw_variant;
    uint8_t fw_revision;
    uint8_t fw_build_num;
    uint8_t fw_build_ww;
    uint8_t fw_build_yy;
    uint8_t fw_patch_num;
} IntelVersion;

typedef struct __attribute__((packed)) {
    uint8_t     result;
    uint16_t   opcode;
    uint8_t     status;
} IntelSecureSendResult;

typedef struct __attribute__((packed)) {
    uint8_t     reset_type;
    uint8_t     patch_enable;
    uint8_t     ddc_reload;
    uint8_t     boot_option;
    uint32_t   boot_param;
} IntelReset;

class BtIntel {
    
public:
    
    static uint8_t intelConvertSpeed(unsigned int speed);
    
    static void printIntelVersion(IntelVersion* ver);
};

#endif /* BtIntel_h */
