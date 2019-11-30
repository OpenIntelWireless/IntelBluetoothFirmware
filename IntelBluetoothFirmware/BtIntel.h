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

typedef struct __attribute__((packed)) {
    uint8_t b[6];
} bdaddr_t;

typedef struct __attribute__((packed)) {
    uint8_t     status;
    uint8_t     otp_format;
    uint8_t     otp_content;
    uint8_t     otp_patch;
    uint16_t   dev_revid;
    uint8_t     secure_boot;
    uint8_t     key_from_hdr;
    uint8_t     key_type;
    uint8_t     otp_lock;
    uint8_t     api_lock;
    uint8_t     debug_lock;
    bdaddr_t   otp_bdaddr;
    uint8_t     min_fw_build_nn;
    uint8_t     min_fw_build_cw;
    uint8_t     min_fw_build_yy;
    uint8_t     limited_cce;
    uint8_t     unlocked_state;
} IntelBootParams;

class BtIntel {
    
public:
    
    static uint8_t intelConvertSpeed(unsigned int speed);
    
    static void printIntelVersion(IntelVersion* ver);
    
    static void printAllByte(void *addr, int size);
};

#endif /* BtIntel_h */
