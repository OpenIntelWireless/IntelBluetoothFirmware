/** @file
 Copyright (c) 2020 zxystd. All rights reserved.
 SPDX-License-Identifier: GPL-3.0-only
 **/

//
//  BtIntel.h
//  IntelBluetoothFirmware
//
//  Created by zxystd on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef BtIntel_h
#define BtIntel_h

#include <libkern/c++/OSObject.h>
#include <libkern/libkern.h>

#include "USBDeviceController.hpp"
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
    uint8_t type;
    uint8_t len;
    uint8_t val[];
} IntelTLV;

typedef struct __attribute__((packed)) {
    uint32_t    cnvi_top;
    uint32_t    cnvr_top;
    uint32_t    cnvi_bt;
    uint32_t    cnvr_bt;
    uint16_t    dev_rev_id;
    uint8_t    img_type;
    uint16_t    timestamp;
    uint8_t    build_type;
    uint32_t    build_num;
    uint8_t    secure_boot;
    uint8_t    otp_lock;
    uint8_t    api_lock;
    uint8_t    debug_lock;
    uint8_t    min_fw_build_nn;
    uint8_t    min_fw_build_cw;
    uint8_t    min_fw_build_yy;
    uint8_t    limited_cce;
    uint8_t    sbe_type;
    bdaddr_t otp_bd_addr;
} IntelVersionTLV;

typedef struct __attribute__((packed)) {
    uint8_t     zero;
    uint8_t     num_cmds;
    uint8_t     source;
    uint8_t     reset_type;
    uint8_t     reset_reason;
    uint8_t     ddc_status;
} IntelBootUp;

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

typedef struct __attribute__((packed)) {
    uint8_t    page1[16];
} IntelDebugFeatures;

typedef struct __attribute__((packed))
{
    uint16_t    opcode;    /* OCF & OGF */
    uint8_t     len;
} FWCommandHdr;

#define HCI_OP_INTEL_VERSION 0xfc05
#define HCI_OP_INTEL_RESET 0xfc52
#define HCI_OP_INTEL_RESET_BOOT 0xfc01

#define HCI_OP_INTEL_ENTER_MFG 0xfc11
#define HCI_OP_READ_INTEL_BOOT_PARAMS 0xfc0d
#define HCI_OP_INTEL_EVENT_MASK 0xfc52

#define INTEL_HW_PLATFORM(cnvx_bt)    ((uint8_t)(((cnvx_bt) & 0x0000ff00) >> 8))
#define INTEL_HW_VARIANT(cnvx_bt)    ((uint8_t)(((cnvx_bt) & 0x003f0000) >> 16))
#define INTEL_CNVX_TOP_TYPE(cnvx_top)    ((cnvx_top) & 0x00000fff)
#define INTEL_CNVX_TOP_STEP(cnvx_top)    (((cnvx_top) & 0x0f000000) >> 24)
#define INTEL_CNVX_TOP_PACK_SWAB(t, s)    OSSwapInt16(((uint16_t)(((t) << 4) | (s))))

#define BDADDR_INTEL        (&(bdaddr_t){{0x00, 0x8b, 0x9e, 0x19, 0x03, 0x00}})
#define RSA_HEADER_LEN        644
#define CSS_HEADER_OFFSET    8
#define ECDSA_OFFSET        644
#define ECDSA_HEADER_LEN    320

#define CMD_BUF_MAX_SIZE    256

class BtIntel : public OSObject {
    OSDeclareAbstractStructors(BtIntel)
public:
    
    virtual bool initWithDevice(IOService *client, IOUSBHostDevice *dev);
    
    virtual void free() override;
    
    virtual bool setup() = 0;
    
    virtual bool shutdown() = 0;
    
    virtual bool getFirmwareName(char *fwname, size_t len) = 0;
    
    bool securedSend(uint8_t fragmentType, uint32_t len, const uint8_t *fragment);
    
    bool enterMfg();
    
    bool exitMfg(bool reset, bool patched);
    
    bool setEventMask(bool debug);
    
    bool setEventMaskMfg(bool debug);
    
    bool readVersion(IntelVersion *version);
    
    bool sendIntelReset(uint32_t bootParam);
    
    bool readBootParams(IntelBootParams *params);
    
    bool resetToBootloader();
    
    bool intelVersionInfo(IntelVersion *ver);
    
    bool intelBoot(uint32_t bootAddr);
    
    bool readDebugFeatures(IntelDebugFeatures *features);
    
    bool setDebugFeatures(IntelDebugFeatures *features);
    
    bool loadDDCConfig(const char *ddcFileName);
    
    OSData *firmwareConvertion(OSData *originalFirmware);
    
    OSData *requestFirmwareData(const char *fwName, bool noWarn = false);
    
    static void printAllByte(void *addr, int size);
    
protected:
    
    bool intelSendHCISync(HciCommandHdr *cmd, void *event, uint32_t eventBufSize, uint32_t *size, int timeout);
    
    bool intelSendHCISyncEvent(HciCommandHdr *cmd, void *event, uint32_t eventBufSize, uint32_t *size, uint8_t syncEvent, int timeout);
    
    bool intelBulkHCISync(HciCommandHdr *cmd, void *event, uint32_t eventBufSize, uint32_t *size, int timeout);
    
protected:
    USBDeviceController *m_pUSBDeviceController;
};

#endif /* BtIntel_h */
