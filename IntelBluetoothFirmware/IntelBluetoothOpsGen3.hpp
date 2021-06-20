//
//  IntelBluetoothOpsGen3.hpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef IntelBluetoothOpsGen3_hpp
#define IntelBluetoothOpsGen3_hpp

#include "IntelBluetoothOpsGen2.hpp"

/* List of tlv type */
enum {
    INTEL_TLV_CNVI_TOP = 0x10,
    INTEL_TLV_CNVR_TOP,
    INTEL_TLV_CNVI_BT,
    INTEL_TLV_CNVR_BT,
    INTEL_TLV_CNVI_OTP,
    INTEL_TLV_CNVR_OTP,
    INTEL_TLV_DEV_REV_ID,
    INTEL_TLV_USB_VENDOR_ID,
    INTEL_TLV_USB_PRODUCT_ID,
    INTEL_TLV_PCIE_VENDOR_ID,
    INTEL_TLV_PCIE_DEVICE_ID,
    INTEL_TLV_PCIE_SUBSYSTEM_ID,
    INTEL_TLV_IMAGE_TYPE,
    INTEL_TLV_TIME_STAMP,
    INTEL_TLV_BUILD_TYPE,
    INTEL_TLV_BUILD_NUM,
    INTEL_TLV_FW_BUILD_PRODUCT,
    INTEL_TLV_FW_BUILD_HW,
    INTEL_TLV_FW_STEP,
    INTEL_TLV_BT_SPEC,
    INTEL_TLV_MFG_NAME,
    INTEL_TLV_HCI_REV,
    INTEL_TLV_LMP_SUBVER,
    INTEL_TLV_OTP_PATCH_VER,
    INTEL_TLV_SECURE_BOOT,
    INTEL_TLV_KEY_FROM_HDR,
    INTEL_TLV_OTP_LOCK,
    INTEL_TLV_API_LOCK,
    INTEL_TLV_DEBUG_LOCK,
    INTEL_TLV_MIN_FW,
    INTEL_TLV_LIMITED_CCE,
    INTEL_TLV_SBE_TYPE,
    INTEL_TLV_OTP_BDADDR,
    INTEL_TLV_UNLOCKED_STATE
};

class IntelBluetoothOpsGen3 : public IntelBluetoothOpsGen2 {
    OSDeclareDefaultStructors(IntelBluetoothOpsGen3)
    
public:
    
    virtual bool setup() override;
    
    virtual bool shutdown() override;
    
    virtual bool getFirmwareName(char *fwname, size_t len) override;
    
protected:
    
    bool ecdsaHeaderSecureSend(OSData *fwData);
    
private:
    
    bool versionInfoTLV(IntelVersionTLV *version);
    
    bool readVersionTLV(IntelVersionTLV *version);
    
    bool getFirmware(IntelVersionTLV *tlv, char *name, size_t len, const char *suffix);
    
    bool downloadFirmware(IntelVersionTLV *ver, uint32_t *bootParams);
    
    IOReturn downloadFirmwareData(IntelVersionTLV *ver, OSData *fwData, uint32_t *bootParams, uint8_t hwVariant, uint8_t sbeType);
    
private:
    char loadedFirmwareName[64];
};

#endif /* IntelBluetoothOpsGen3_hpp */
