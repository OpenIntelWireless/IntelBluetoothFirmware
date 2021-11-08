//
//  IntelBluetoothOpsGen3.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "IntelBluetoothOpsGen3.hpp"
#include "Log.h"
#include "linux.h"

OSDefineMetaClassAndStructors(IntelBluetoothOpsGen3, IntelBluetoothOpsGen2)

bool IntelBluetoothOpsGen3::
setup()
{
    char ddcname[64];
    uint32_t bootParams;
    IntelDebugFeatures features;
    IntelVersionTLV version;
    bool ret = true;
    
    /* Set the default boot parameter to 0x0 and it is updated to
     * SKU specific boot parameter after reading Intel_Write_Boot_Params
     * command while downloading the firmware.
     */
    bootParams = 0x00000000;
    
    /* Read the Intel version information to determine if the device
     * is in bootloader mode or if it already has operational firmware
     * loaded.
     */
    if (!readVersionTLV(&version)) {
        XYLog("Intel Read version failed %d\n", __LINE__);
        return false;
    }
    
    if (!versionInfoTLV(&version)) {
        XYLog("Intel version check failed\n");
        return false;
    }
    
    if (!downloadFirmware(&version, &bootParams)) {
        XYLog("Download firmware failed\n");
        return false;
    }
    
    /* check if controller is already having an operational firmware */
    if (version.img_type == 0x03) {
        XYLog("Frimware is already running, finishing\n");
        goto finish;
    }
    
    if (!intelBoot(bootParams)) {
        XYLog("Boot failed\n");
        return false;
    }
    
    getFirmware(&version, ddcname, sizeof(ddcname), "ddc");
    
    /* Once the device is running in operational mode, it needs to
     * apply the device configuration (DDC) parameters.
     *
     * The device can work without DDC parameters, so even if it
     * fails to load the file, no need to fail the setup.
     */
    loadDDCConfig(ddcname);
    
    /* Read the Intel supported features and if new exception formats
     * supported, need to load the additional DDC config to enable.
     */
    readDebugFeatures(&features);
    
    /* Set DDC mask for available debug features */
    setDebugFeatures(&features);
    
    /* Read the Intel version information after loading the FW  */
    if (!readVersionTLV(&version)) {
        XYLog("Intel Read version failed %d\n", __LINE__);
        return false;
    }
    
    versionInfoTLV(&version);
    
finish:
    /* Set the event mask for Intel specific vendor events. This enables
     * a few extra events that are useful during general operation. It
     * does not enable any debugging related events.
     *
     * The device will function correctly without these events enabled
     * and thus no need to fail the setup.
     */
    setEventMask(false);
    
    return ret;
}

bool IntelBluetoothOpsGen3::
downloadFirmware(IntelVersionTLV *ver, uint32_t *bootParams)
{
    char fwname[64];
    OSData *fwData = NULL;
    IOReturn ior;
    uint32_t actSize = 0;
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciResponse *resp = (HciResponse *)buf;
    bool firmwareMode = false;
    bool ret = true;
    
    if (!ver || !bootParams) {
        return false;
    }
    
    /* The firmware variant determines if the device is in bootloader
     * mode or is running operational firmware. The value 0x03 identifies
     * the bootloader and the value 0x23 identifies the operational
     * firmware.
     *
     * When the operational firmware is already present, then only
     * the check for valid Bluetooth device address is needed. This
     * determines if the device will be added as configured or
     * unconfigured controller.
     *
     * It is not possible to use the Secure Boot Parameters in this
     * case since that command is only available in bootloader mode.
     */
    if (ver->img_type == 0x03) {
        firmwareMode = true;
    }
    
    getFirmware(ver, fwname, sizeof(fwname), "sfi");
    
    strncpy(this->loadedFirmwareName, fwname, sizeof(this->loadedFirmwareName));
    
    fwData = requestFirmwareData(fwname, true);
    if (!fwData) {
        if (firmwareMode) {
            /* Firmware has already been loaded */
            return true;
        }
        XYLog("Failed to load Intel firmware file %s\n", fwname);
        return false;
    }
    
    XYLog("Found device firmware: %s\n", fwname);
    
    if (fwData->getLength() < 644) {
        XYLog("Invalid size of firmware file (%zu)",
              (size_t)fwData->getLength());
        ret = false;
        goto done;
    }
    
    /* Start firmware downloading and get boot parameter */
    if ((ior = downloadFirmwareData(ver, fwData, bootParams,
                                    INTEL_HW_VARIANT(ver->cnvi_bt), ver->sbe_type)) != kIOReturnSuccess) {
        if (ior == -EALREADY) {
            /* Firmware has already been loaded */
            ret = true;
            goto done;
        }
        
        /* When FW download fails, send Intel Reset to retry
         * FW download.
         */
        resetToBootloader();
        goto done;
    }
    
    /* Before switching the device into operational mode and with that
     * booting the loaded firmware, wait for the bootloader notification
     * that all fragments have been successfully received.
     *
     * When the event processing receives the notification, then the
     * BTUSB_DOWNLOADING flag will be cleared.
     *
     * The firmware loading should not take longer than 5 seconds
     * and thus just timeout if that happens and fail the setup
     * of this device.
     */
    memset(buf, 0, sizeof(buf));
    ior = m_pUSBDeviceController->interruptPipeRead(resp, sizeof(buf), &actSize, 5000);
    if (ior != kIOReturnSuccess) {
        XYLog("waiting for firmware download done timeout\n");
        resetToBootloader();
        ret = false;
        goto done;
    }
    
    if (resp->evt.evt == 0xff && resp->numCommands == 0x06) {
        XYLog("Notify: Firmware download done\n");
        goto done;
    }
    
    ret = false;
    
done:
    OSSafeReleaseNULL(fwData);
    return ret;
}

IOReturn IntelBluetoothOpsGen3::
downloadFirmwareData(IntelVersionTLV *ver, OSData *fwData, uint32_t *bootParams, uint8_t hwVariant, uint8_t sbeType)
{
    uint32_t cssHeaderVer;

    /* Skip download if firmware has the same version */
    if (firmwareVersion(ver->min_fw_build_nn,
                        ver->min_fw_build_cw,
                        ver->min_fw_build_yy,
                        fwData, bootParams)) {
        XYLog("Firmware already loaded\n");
        /* Return -EALREADY to indicate that firmware has
         * already been loaded.
         */
        return -EALREADY;
    }
    
    /* The firmware variant determines if the device is in bootloader
     * mode or is running operational firmware. The value 0x01 identifies
     * the bootloader and the value 0x03 identifies the operational
     * firmware.
     *
     * If the firmware version has changed that means it needs to be reset
     * to bootloader when operational so the new firmware can be loaded.
     */
    if (ver->img_type == 0x03)
        return kIOReturnError;
    
    /* iBT hardware variants 0x0b, 0x0c, 0x11, 0x12, 0x13, 0x14 support
     * only RSA secure boot engine. Hence, the corresponding sfi file will
     * have RSA header of 644 bytes followed by Command Buffer.
     *
     * iBT hardware variants 0x17, 0x18 onwards support both RSA and ECDSA
     * secure boot engine. As a result, the corresponding sfi file will
     * have RSA header of 644, ECDSA header of 320 bytes followed by
     * Command Buffer.
     *
     * CSS Header byte positions 0x08 to 0x0B represent the CSS Header
     * version: RSA(0x00010000) , ECDSA (0x00020000)
     */
    cssHeaderVer = get_unaligned_le32((uint8_t *)fwData->getBytesNoCopy() + CSS_HEADER_OFFSET);
    
    if (cssHeaderVer != 0x00010000) {
        XYLog("Invalid CSS Header version: %d %d\n", cssHeaderVer, __LINE__);
        return kIOReturnError;
    }
    
    XYLog("%s hwVariant: %d sbeType: %d\n", __FUNCTION__, hwVariant, sbeType);
    
    if (hwVariant <= 0x14) {
        if (sbeType != 0x00) {
            XYLog("Invalid SBE type for hardware variant (%d)",
                  hwVariant);
            return kIOReturnError;
        }
        
        if (!rsaHeaderSecureSend(fwData)) {
            XYLog("Send RSA header failed\n");
            return kIOReturnError;
        }
        
        if (!downloadFirmwarePayload(fwData, RSA_HEADER_LEN)) {
            return kIOReturnError;
        }
        
    } else if (hwVariant >= 0x17) {
        /* Check if CSS header for ECDSA follows the RSA header */
        if (((uint8_t *)fwData->getBytesNoCopy())[ECDSA_OFFSET] != 0x06)
            return -EINVAL;
        
        /* Check if the CSS Header version is ECDSA(0x00020000) */
        cssHeaderVer = get_unaligned_le32((uint8_t *)fwData->getBytesNoCopy() + ECDSA_OFFSET + CSS_HEADER_OFFSET);
        if (cssHeaderVer != 0x00020000) {
            XYLog("Invalid CSS Header version: %d %d\n", cssHeaderVer, __LINE__);
            return kIOReturnError;
        }
        
        if (sbeType == 0x00) {
            if (!rsaHeaderSecureSend(fwData)) {
                XYLog("Send RSA header failed\n");
                return kIOReturnError;
            }
            
            if (!downloadFirmwarePayload(fwData, RSA_HEADER_LEN + ECDSA_HEADER_LEN)) {
                return kIOReturnError;
            }
        } else if (sbeType == 0x01) {
            if (!ecdsaHeaderSecureSend(fwData)) {
                XYLog("Send ECDSA header failed\n");
                return kIOReturnError;
            }
            
            if (!downloadFirmwarePayload(fwData, RSA_HEADER_LEN + ECDSA_HEADER_LEN)) {
                return kIOReturnError;
            }
        }
    }
    
    return kIOReturnSuccess;
}

bool IntelBluetoothOpsGen3::
ecdsaHeaderSecureSend(OSData *fwData)
{
    /* Start the firmware download transaction with the Init fragment
     * represented by the 128 bytes of CSS header.
     */
    XYLog("send firmware header\n");
    if (!securedSend(0x00, 128, (const uint8_t *)fwData->getBytesNoCopy() + 644)) {
        XYLog("Failed to send firmware header\n");
        return false;
    }
    XYLog("send firmware header done\n");
    
    /* Send the 96 bytes of public key information from the firmware
     * as the PKey fragment.
     */
    XYLog("send firmware pkey\n");
    if (!securedSend(0x03, 96, (const uint8_t *)fwData->getBytesNoCopy() + 644 + 128)) {
        XYLog("Failed to send firmware pkey\n");
        return false;
    }
    XYLog("send firmware pkey done\n");
    
    /* Send the 96 bytes of signature information from the firmware
     * as the Sign fragment
     */
    XYLog("send firmware signature\n");
    if (!securedSend(0x02, 96, (const uint8_t *)fwData->getBytesNoCopy() + 644 + 224)) {
        XYLog("Failed to send firmware signature\n");
        return false;
    }
    XYLog("send firmware signature done\n");
    
    return true;
}

bool IntelBluetoothOpsGen3::
readVersionTLV(IntelVersionTLV *version)
{
    uint32_t actLen = 0;
    uint8_t sendBuf[CMD_BUF_MAX_SIZE];
    uint8_t respBuf[CMD_BUF_MAX_SIZE];
    HciResponse *resp = (HciResponse *)respBuf;
    HciCommandHdr *cmd = (HciCommandHdr *)sendBuf;
    const uint8_t *versionDataPtr;
    IntelTLV *tlv;
    int len = 0;
    
    memset(sendBuf, 0, sizeof(sendBuf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfc05);
    cmd->len = 1;
    cmd->data[0] = 0xFF;
    
    memset(respBuf, 0, sizeof(respBuf));
    if (!intelSendHCISync(cmd, resp, sizeof(respBuf), &actLen, HCI_CMD_TIMEOUT)) {
        XYLog("Reading Intel version information failed\n");
        return false;
    }
    
    if (actLen < 5) {
        XYLog("Invalid size %d\n", actLen);
        return false;
    }
    
    versionDataPtr = resp->data;
    len = actLen - 5;
    
    /* Consume Command Complete Status field */
    versionDataPtr++;
    len--;
    
    /* Event parameters contatin multiple TLVs. Read each of them
     * and only keep the required data. Also, it use existing legacy
     * version field like hw_platform, hw_variant, and fw_variant
     * to keep the existing setup flow
     */
    while (len > 0) {
        tlv = (IntelTLV *)versionDataPtr;
        switch (tlv->type) {
            case INTEL_TLV_CNVI_TOP:
                version->cnvi_top = get_unaligned_le32(tlv->val);
                break;
            case INTEL_TLV_CNVR_TOP:
                version->cnvr_top = get_unaligned_le32(tlv->val);
                break;
            case INTEL_TLV_CNVI_BT:
                version->cnvi_bt = get_unaligned_le32(tlv->val);
                break;
            case INTEL_TLV_CNVR_BT:
                version->cnvr_bt = get_unaligned_le32(tlv->val);
                break;
            case INTEL_TLV_DEV_REV_ID:
                version->dev_rev_id = get_unaligned_le16(tlv->val);
                break;
            case INTEL_TLV_IMAGE_TYPE:
                version->img_type = tlv->val[0];
                break;
            case INTEL_TLV_TIME_STAMP:
                /* If image type is Operational firmware (0x03), then
                 * running FW Calendar Week and Year information can
                 * be extracted from Timestamp information
                 */
                version->min_fw_build_cw = tlv->val[0];
                version->min_fw_build_yy = tlv->val[1];
                version->timestamp = get_unaligned_le16(tlv->val);
                break;
            case INTEL_TLV_BUILD_TYPE:
                version->build_type = tlv->val[0];
                break;
            case INTEL_TLV_BUILD_NUM:
                /* If image type is Operational firmware (0x03), then
                 * running FW build number can be extracted from the
                 * Build information
                 */
                version->min_fw_build_nn = tlv->val[0];
                version->build_num = get_unaligned_le32(tlv->val);
                break;
            case INTEL_TLV_SECURE_BOOT:
                version->secure_boot = tlv->val[0];
                break;
            case INTEL_TLV_OTP_LOCK:
                version->otp_lock = tlv->val[0];
                break;
            case INTEL_TLV_API_LOCK:
                version->api_lock = tlv->val[0];
                break;
            case INTEL_TLV_DEBUG_LOCK:
                version->debug_lock = tlv->val[0];
                break;
            case INTEL_TLV_MIN_FW:
                version->min_fw_build_nn = tlv->val[0];
                version->min_fw_build_cw = tlv->val[1];
                version->min_fw_build_yy = tlv->val[2];
                break;
            case INTEL_TLV_LIMITED_CCE:
                version->limited_cce = tlv->val[0];
                break;
            case INTEL_TLV_SBE_TYPE:
                version->sbe_type = tlv->val[0];
                break;
            case INTEL_TLV_OTP_BDADDR:
                memcpy(&version->otp_bd_addr, tlv->val, tlv->len);
                break;
            default:
                /* Ignore rest of information */
                break;
        }
        
        len -= (sizeof(IntelTLV) + tlv->len);
        versionDataPtr += (sizeof(IntelTLV) + tlv->len);
    }
    
    return true;
}

bool IntelBluetoothOpsGen3::
versionInfoTLV(IntelVersionTLV *version)
{
    const char *variant;
    
    /* The hardware platform number has a fixed value of 0x37 and
     * for now only accept this single value.
     */
    if (INTEL_HW_PLATFORM(version->cnvi_bt) != 0x37) {
        XYLog("Unsupported Intel hardware platform (0x%2x)\n",
              INTEL_HW_PLATFORM(version->cnvi_bt));
        return false;
    }
    
    /* Check for supported iBT hardware variants of this firmware
     * loading method.
     *
     * This check has been put in place to ensure correct forward
     * compatibility options when newer hardware variants come along.
     */
    switch (INTEL_HW_VARIANT(version->cnvi_bt)) {
        case 0x17:    /* TyP */
        case 0x18:    /* Slr */
        case 0x19:    /* Slr-F */
            break;
        default:
            XYLog("Unsupported Intel hardware variant (0x%x)\n",
                  INTEL_HW_VARIANT(version->cnvi_bt));
            return false;
    }
    
    switch (version->img_type) {
        case 0x01:
            variant = "Bootloader";
            /* It is required that every single firmware fragment is acknowledged
             * with a command complete event. If the boot parameters indicate
             * that this bootloader does not send them, then abort the setup.
             */
            if (version->limited_cce != 0x00) {
                XYLog("Unsupported Intel firmware loading method (0x%x)\n",
                      version->limited_cce);
                return false;
            }
            
            /* Secure boot engine type should be either 1 (ECDSA) or 0 (RSA) */
            if (version->sbe_type > 0x01) {
                XYLog("Unsupported Intel secure boot engine type (0x%x)\n",
                      version->sbe_type);
                return false;
            }
            
            XYLog("Device revision is %u\n", version->dev_rev_id);
            XYLog("Secure boot is %s\n",
                  version->secure_boot ? "enabled" : "disabled");
            XYLog("OTP lock is %s\n",
                  version->otp_lock ? "enabled" : "disabled");
            XYLog("API lock is %s\n",
                  version->api_lock ? "enabled" : "disabled");
            XYLog("Debug lock is %s\n",
                  version->debug_lock ? "enabled" : "disabled");
            XYLog("Minimum firmware build %u week %u %u\n",
                  version->min_fw_build_nn, version->min_fw_build_cw,
                  2000 + version->min_fw_build_yy);
            break;
        case 0x03:
            variant = "Firmware";
            break;
        default:
            XYLog("Unsupported image type(%02x)\n", version->img_type);
            return false;
    }
    
    XYLog("%s timestamp %u.%u buildtype %u build %u\n", variant,
          2000 + (version->timestamp >> 8), version->timestamp & 0xff,
          version->build_type, version->build_num);
    
    return true;
}

bool IntelBluetoothOpsGen3::
getFirmware(IntelVersionTLV *tlv, char *name, size_t len, const char *suffix)
{
    /* The firmware file name for new generation controllers will be
     * ibt-<cnvi_top type+cnvi_top step>-<cnvr_top type+cnvr_top step>
     */
    snprintf(name, len, "ibt-%04x-%04x.%s",
             INTEL_CNVX_TOP_PACK_SWAB(INTEL_CNVX_TOP_TYPE(tlv->cnvi_top),
                                      INTEL_CNVX_TOP_STEP(tlv->cnvi_top)),
             INTEL_CNVX_TOP_PACK_SWAB(INTEL_CNVX_TOP_TYPE(tlv->cnvr_top),
                                      INTEL_CNVX_TOP_STEP(tlv->cnvr_top)),
             suffix);
    return true;
}

bool IntelBluetoothOpsGen3::
shutdown()
{
    return true;
}

bool IntelBluetoothOpsGen3::
getFirmwareName(char *fwname, size_t len)
{
    strncpy(fwname, this->loadedFirmwareName, len - 1);
    fwname[len - 1] = '\0';
    return true;
}
