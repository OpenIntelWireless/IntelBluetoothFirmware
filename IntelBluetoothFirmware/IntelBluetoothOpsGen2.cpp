//
//  IntelBluetoothOpsGen2.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "IntelBluetoothOpsGen2.hpp"
#include "Log.h"

OSDefineMetaClassAndStructors(IntelBluetoothOpsGen2, BtIntel)

bool IntelBluetoothOpsGen2::
setup()
{
    IntelVersion ver;
    IntelBootParams params;
    IntelDebugFeatures features;
    uint32_t bootParams;
    char ddcname[64];
    
    /* Set the default boot parameter to 0x0 and it is updated to
     * SKU specific boot parameter after reading Intel_Write_Boot_Params
     * command while downloading the firmware.
     */
    bootParams = 0x00000000;
    
    /* Read the Intel version information to determine if the device
     * is in bootloader mode or if it already has operational firmware
     * loaded.
     */
    if (!readVersion(&ver)) {
        XYLog("Intel Read version failed\n");
        resetToBootloader();
        return false;
    }
    
    if (!intelVersionInfo(&ver)) {
        return false;
    }
    
    if (!downloadFirmware(&ver, &params, &bootParams)) {
        return false;
    }
    
    /* controller is already having an operational firmware */
    if (ver.fw_variant == 0x23) {
        XYLog("Frimware is already running, finishing\n");
        goto finish;
    }
    
    if (!intelBoot(bootParams)) {
        XYLog("Boot failed\n");
        return false;
    }
    
    if (!getFirmware(&ver, &params, ddcname, sizeof(ddcname), "ddc")) {
        XYLog("Unsupported Intel firmware naming\n");
    } else {
        /* Once the device is running in operational mode, it needs to
         * apply the device configuration (DDC) parameters.
         *
         * The device can work without DDC parameters, so even if it
         * fails to load the file, no need to fail the setup.
         */
        loadDDCConfig(ddcname);
    }
    
    /* Read the Intel supported features and if new exception formats
     * supported, need to load the additional DDC config to enable.
     */
    readDebugFeatures(&features);
    
    /* Set DDC mask for available debug features */
    setDebugFeatures(&features);
    
    /* Read the Intel version information after loading the FW  */
    if (!readVersion(&ver)) {
        return false;
    }
    
    intelVersionInfo(&ver);
    
finish:
    
    /* Set the event mask for Intel specific vendor events. This enables
     * a few extra events that are useful during general operation. It
     * does not enable any debugging related events.
     *
     * The device will function correctly without these events enabled
     * and thus no need to fail the setup.
     */
    setEventMask(false);
    
    return true;
}

bool IntelBluetoothOpsGen2::
downloadFirmware(IntelVersion *ver, IntelBootParams *params, uint32_t *bootParams)
{
    OSData *fwData;
    char fwname[64];
    IOReturn ior;
    uint32_t actSize = 0;
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciResponse *resp = (HciResponse *)buf;
    bool firmwareMode = false;
    bool ret = true;
    
    if (!ver || !params) {
        return false;
    }
    
    /* The firmware variant determines if the device is in bootloader
     * mode or is running operational firmware. The value 0x06 identifies
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
    if (ver->fw_variant == 0x23) {
        
        firmwareMode = true;
        
        /* SfP and WsP don't seem to update the firmware version on file
         * so version checking is currently possible.
         */
        switch (ver->hw_variant) {
            case 0x0b:    /* SfP */
            case 0x0c:    /* WsP */
                return true;
        }
        
        /* Proceed to download to check if the version matches */
        goto download;
    }
    
    /* Read the secure boot parameters to identify the operating
     * details of the bootloader.
     */
    if (!readBootParams(params)) {
        return false;
    }
    
    /* It is required that every single firmware fragment is acknowledged
     * with a command complete event. If the boot parameters indicate
     * that this bootloader does not send them, then abort the setup.
     */
    if (params->limited_cce != 0x00) {
        XYLog("Unsupported Intel firmware loading method (%u)\n",
              params->limited_cce);
        return false;
    }
    
download:
    /* With this Intel bootloader only the hardware variant and device
     * revision information are used to select the right firmware for SfP
     * and WsP.
     *
     * The firmware filename is ibt-<hw_variant>-<dev_revid>.sfi.
     *
     * Currently the supported hardware variants are:
     *   11 (0x0b) for iBT3.0 (LnP/SfP)
     *   12 (0x0c) for iBT3.5 (WsP)
     *
     * For ThP/JfP and for future SKU's, the FW name varies based on HW
     * variant, HW revision and FW revision, as these are dependent on CNVi
     * and RF Combination.
     *
     *   17 (0x11) for iBT3.5 (JfP)
     *   18 (0x12) for iBT3.5 (ThP)
     *
     * The firmware file name for these will be
     * ibt-<hw_variant>-<hw_revision>-<fw_revision>.sfi.
     *
     */
    if (!getFirmware(ver, params, fwname, sizeof(fwname), "sfi")) {
        if (firmwareMode) {
            /* Firmware has already been loaded */
            return true;
        }
        XYLog("Unsupported Intel firmware naming\n");
        return false;
    }
    
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
    
    if (fwData->getLength() < 644) {
        XYLog("Invalid size of firmware file (%zu)\n",
              (size_t)fwData->getLength());
        ret = false;
        goto done;
    }
    
    /* Start firmware downloading and get boot parameter */
    if ((ior = downloadFirmwareData(ver, fwData, bootParams)) != kIOReturnSuccess) {
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

IOReturn IntelBluetoothOpsGen2::
downloadFirmwareData(IntelVersion *ver, OSData *fwData, uint32_t *bootParams)
{
    /* SfP and WsP don't seem to update the firmware version on file
     * so version checking is currently not possible.
     */
    switch (ver->hw_variant) {
        case 0x0b:    /* SfP */
        case 0x0c:    /* WsP */
            /* Skip version checking */
            break;
        default:
            
            /* Skip download if firmware has the same version */
            if (firmwareVersion(ver->fw_build_num,
                                ver->fw_build_ww, ver->fw_build_yy,
                                fwData, bootParams)) {
                XYLog("Firmware already loaded\n");
                /* Return -EALREADY to indicate that the firmware has
                 * already been loaded.
                 */
                return -EALREADY;
            }
    }
    
    /* The firmware variant determines if the device is in bootloader
     * mode or is running operational firmware. The value 0x06 identifies
     * the bootloader and the value 0x23 identifies the operational
     * firmware.
     *
     * If the firmware version has changed that means it needs to be reset
     * to bootloader when operational so the new firmware can be loaded.
     */
    if (ver->fw_variant == 0x23) {
        XYLog("Unsupported fw variant: %d\n", ver->fw_variant);
        return kIOReturnInvalid;
    }
    
    if (!rsaHeaderSecureSend(fwData)) {
        XYLog("Send RSA header failed\n");
        return kIOReturnError;
    }
    
    return downloadFirmwarePayload(fwData, RSA_HEADER_LEN) ? kIOReturnSuccess : kIOReturnError;
}

bool IntelBluetoothOpsGen2::
getFirmware(IntelVersion *ver, IntelBootParams *params, 
            char *name, size_t len, const char *suffix)
{
    switch (ver->hw_variant) {
        case 0x0b:    /* SfP */
        case 0x0c:    /* WsP */
            snprintf(name, len, "ibt-%u-%u.%s",
                     OSSwapLittleToHostInt16(ver->hw_variant),
                     OSSwapLittleToHostInt16(params->dev_revid),
                     suffix);
            break;
        case 0x11:    /* JfP */
        case 0x12:    /* ThP */
        case 0x13:    /* HrP */
        case 0x14:    /* CcP */
            snprintf(name, len, "ibt-%u-%u-%u.%s",
                     OSSwapLittleToHostInt16(ver->hw_variant),
                     OSSwapLittleToHostInt16(ver->hw_revision),
                     OSSwapLittleToHostInt16(ver->fw_revision),
                     suffix);
            break;
        default:
            return false;
    }
    
    return true;
}

bool IntelBluetoothOpsGen2::
rsaHeaderSecureSend(OSData *fwData)
{
    /* Start the firmware download transaction with the Init fragment
     * represented by the 128 bytes of CSS header.
     */
    XYLog("send firmware header\n");
    if (!securedSend(0x00, 128, (const uint8_t *)fwData->getBytesNoCopy())) {
        XYLog("Failed to send firmware header\n");
        return false;
    }
    XYLog("send firmware header done\n");
    
    /* Send the 256 bytes of public key information from the firmware
     * as the PKey fragment.
     */
    XYLog("send firmware pkey\n");
    if (!securedSend(0x03, 256, (const uint8_t *)fwData->getBytesNoCopy() + 128)) {
        XYLog("Failed to send firmware pkey\n");
        return false;
    }
    XYLog("send firmware pkey done\n");
    
    /* Send the 256 bytes of signature information from the firmware
     * as the Sign fragment.
     */
    XYLog("send firmware signature\n");
    if (!securedSend(0x02, 256, (const uint8_t *)fwData->getBytesNoCopy() + 388)) {
        XYLog("Failed to send firmware signature\n");
        return false;
    }
    XYLog("send firmware signature done\n");
    
    return true;
}

bool IntelBluetoothOpsGen2::
downloadFirmwarePayload(OSData *fwData, size_t offset)
{
    XYLog("send firmware payload\n");
    uint32_t frag_len;
    bool ret = true;
    const uint8_t *fw_ptr = (uint8_t *)fwData->getBytesNoCopy() + offset;
    frag_len = 0;
    
    while (fw_ptr - (uint8_t *)fwData->getBytesNoCopy() < fwData->getLength()) {
        HciCommandHdr *cmd = (HciCommandHdr *)(fw_ptr + frag_len);
        
        frag_len += sizeof(*cmd) + cmd->len;
        
        /* The parameter length of the secure send command requires
         * a 4 byte alignment. It happens so that the firmware file
         * contains proper Intel_NOP commands to align the fragments
         * as needed.
         *
         * Send set of commands with 4 byte alignment from the
         * firmware data buffer as a single Data fragement.
         */
        if (!(frag_len % 4)) {
            if (!securedSend(0x01, frag_len, fw_ptr)) {
                XYLog("Failed to send firmware data\n");
                ret = false;
                goto done;
            }
            
            fw_ptr += frag_len;
            frag_len = 0;
        }
    }
    
done:
    XYLog("send firmware payload done\n");
    return ret;
}

bool IntelBluetoothOpsGen2::
firmwareVersion(uint8_t num, uint8_t ww, uint8_t yy, OSData *fwData, uint32_t *bootAddr)
{
    XYLog("%s\n", __FUNCTION__);
    const uint8_t *fw_ptr = (const uint8_t *)fwData->getBytesNoCopy();
    
    while (fw_ptr - (uint8_t *)fwData->getBytesNoCopy() < fwData->getLength()) {
        FWCommandHdr *cmd = (FWCommandHdr *)(fw_ptr);

        /* Each SKU has a different reset parameter to use in the
         * HCI_Intel_Reset command and it is embedded in the firmware
         * data. So, instead of using static value per SKU, check
         * the firmware data and save it for later use.
         */
        if (OSSwapLittleToHostConstInt16(cmd->opcode) == CMD_WRITE_BOOT_PARAMS) {
            struct cmd_write_boot_params *params;

            params = (struct cmd_write_boot_params *)(fw_ptr + sizeof(*cmd));
            
            *bootAddr = OSSwapLittleToHostConstInt32(params->boot_addr);
            
            XYLog("Boot Address: 0x%x\n", *bootAddr);

            XYLog("Firmware Version: %u-%u.%u\n",
                    params->fw_build_num, params->fw_build_ww,
                    params->fw_build_yy);

            return (num == params->fw_build_num &&
                ww == params->fw_build_ww &&
                yy == params->fw_build_yy);
        }

        fw_ptr += sizeof(*cmd) + cmd->len;
    }

    return false;
}

bool IntelBluetoothOpsGen2::
shutdown()
{
    return true;
}

bool IntelBluetoothOpsGen2::
getFirmwareName(char *fwname, size_t len)
{
    strncpy(fwname, this->loadedFirmwareName, len - 1);
    fwname[len - 1] = '\0';
    return true;
}
