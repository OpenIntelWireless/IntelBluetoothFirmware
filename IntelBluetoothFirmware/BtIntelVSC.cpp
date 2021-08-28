//
//  BtIntelVSC.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/16.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "Log.h"
#include "BtIntel.h"

bool BtIntel::
enterMfg()
{
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    
    memset(buf, 0, sizeof(buf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfc11);
    cmd->len = 2;
    cmd->data[0] = 0x01;
    cmd->data[1] = 0x00;
    
    return intelSendHCISync(cmd, NULL, 0, NULL, HCI_CMD_TIMEOUT);
}

bool BtIntel::
exitMfg(bool reset, bool patched)
{
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    
    memset(buf, 0, sizeof(buf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfc11);
    cmd->len = 2;
    cmd->data[0] = 0x00;
    cmd->data[1] = 0x00;
    
    /* The 2nd command parameter specifies the manufacturing exit method:
     * 0x00: Just disable the manufacturing mode (0x00).
     * 0x01: Disable manufacturing mode and reset with patches deactivated.
     * 0x02: Disable manufacturing mode and reset with patches activated.
     */
    if (reset)
        cmd->data[1] |= patched ? 0x02 : 0x01;
    
    return intelSendHCISync(cmd, NULL, 0, NULL, HCI_CMD_TIMEOUT);
}

bool BtIntel::
setEventMask(bool debug)
{
    uint8_t buf[CMD_BUF_MAX_SIZE];
    uint8_t mask[8] = { 0x87, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    
    if (debug)
        mask[1] |= 0x62;
    memset(buf, 0, sizeof(buf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfc52);
    cmd->len = 8;
    memcpy(cmd->data, mask, 8);
    
    return intelSendHCISync(cmd, NULL, 0, NULL, HCI_INIT_TIMEOUT);
}

bool BtIntel::
setEventMaskMfg(bool debug)
{
    bool ret;
    if (!enterMfg()) {
        return false;
    }
    
    ret = setEventMask(debug);
    
    if (!exitMfg(false, false)) {
        return false;
    }
    
    return ret;
}

bool BtIntel::
readVersion(IntelVersion *version)
{
    uint32_t actLen = 0;
    HciCommandHdr cmd = {
        .opcode = OSSwapHostToLittleInt16(0xfc05),
        .len = 0,
    };
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciResponse *resp = (HciResponse *)buf;
    
    memset(buf, 0, sizeof(buf));
    if (!intelSendHCISync(&cmd, resp, sizeof(buf), &actLen, HCI_CMD_TIMEOUT)) {
        XYLog("Reading Intel version information failed\n");
        return false;
    }
    
    if (actLen - 5 != sizeof(*version)) {
        XYLog("Intel version event size mismatch (act: %d, ver: %d)\n", actLen, (int)sizeof(*version));
        return false;
    }
    
    memcpy(version, resp->data, actLen - 5);
    
    return true;
}

bool BtIntel::
sendIntelReset(uint32_t bootParam)
{
    uint8_t buf[CMD_BUF_MAX_SIZE];
    IntelReset params = {
        0x00, 0x01, 0x00, 0x01, 0x00000000 
    };
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    
    params.boot_param = OSSwapHostToLittleInt32(bootParam);
    
    cmd->opcode = OSSwapHostToLittleInt16(0xfc01);
    cmd->len = sizeof(params);
    memcpy(cmd->data, &params, sizeof(params));
    return m_pUSBDeviceController->sendHCIRequest(cmd, HCI_INIT_TIMEOUT) == kIOReturnSuccess;
}

bool BtIntel::
readBootParams(IntelBootParams *params)
{
    uint32_t actLen = 0;
    HciCommandHdr cmd = {
        .opcode = OSSwapHostToLittleInt16(0xfc0d),
        .len = 0,
    };
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciResponse *resp = (HciResponse *)buf;
    
    memset(buf, 0, sizeof(buf));
    if (!intelSendHCISync(&cmd, resp, sizeof(buf), &actLen, HCI_INIT_TIMEOUT)) {
        XYLog("Reading Intel boot parameters failed\n");
        return false;
    }
    
    if (actLen - 5 != sizeof(*params)) {
        XYLog("Intel boot parameters size mismatch\n");
        return false;
    }
    
    memcpy(params, resp->data, actLen - 5);
    
    if (params->status) {
        XYLog("Intel boot parameters command failed (%02x)\n", params->status);
        return false;
    }
    
    XYLog("Device revision is %u\n",
            OSSwapLittleToHostInt16(params->dev_revid));

    XYLog("Secure boot is %s\n",
            params->secure_boot ? "enabled" : "disabled");

    XYLog("OTP lock is %s\n",
            params->otp_lock ? "enabled" : "disabled");

    XYLog("API lock is %s\n",
            params->api_lock ? "enabled" : "disabled");

    XYLog("Debug lock is %s\n",
            params->debug_lock ? "enabled" : "disabled");

    XYLog("Minimum firmware build %u week %u %u\n",
            params->min_fw_build_nn, params->min_fw_build_cw,
            2000 + params->min_fw_build_yy);
    
    return true;
}

bool BtIntel::
resetToBootloader()
{
    XYLog("%s\n", __FUNCTION__);
    bool ret;
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    IntelReset params;
    
    /* Send Intel Reset command. This will result in
     * re-enumeration of BT controller.
     *
     * Intel Reset parameter description:
     * reset_type :   0x00 (Soft reset),
     *          0x01 (Hard reset)
     * patch_enable : 0x00 (Do not enable),
     *          0x01 (Enable)
     * ddc_reload :   0x00 (Do not reload),
     *          0x01 (Reload)
     * boot_option:   0x00 (Current image),
     *                0x01 (Specified boot address)
     * boot_param:    Boot address
     *
     */
    params.reset_type = 0x01;
    params.patch_enable = 0x01;
    params.ddc_reload = 0x01;
    params.boot_option = 0x00;
    params.boot_param = OSSwapHostToLittleInt32(0x00000000);
    
    cmd->opcode = OSSwapHostToLittleInt16(0xfc01);
    cmd->len = sizeof(params);
    memcpy(cmd->data, &params, sizeof(params));
    
    ret = m_pUSBDeviceController->sendHCIRequest(cmd, HCI_INIT_TIMEOUT);
    
    /* Current Intel BT controllers(ThP/JfP) hold the USB reset
     * lines for 2ms when it receives Intel Reset in bootloader mode.
     * Whereas, the upcoming Intel BT controllers will hold USB reset
     * for 150ms. To keep the delay generic, 150ms is chosen here.
     */
    IOSleep(150);
    
    return ret;
}

bool BtIntel::
readDebugFeatures(IntelDebugFeatures *features)
{
    uint8_t buf[CMD_BUF_MAX_SIZE], temp[CMD_BUF_MAX_SIZE];
    uint actLen = 0;
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    HciResponse *resp = (HciResponse *)temp;
    uint8_t page_no = 1;
    
    memset(buf, 0, sizeof(buf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfca6);
    cmd->len = sizeof(page_no);
    cmd->data[0] = page_no;
    
    if (!intelSendHCISync(cmd, resp, sizeof(temp), &actLen, HCI_INIT_TIMEOUT)) {
        XYLog("Reading supported features failed\n");
        return false;
    }
    
    if (actLen - 5 != sizeof(features->page1) + 3) {
        XYLog("Supported features event size mismatch\n");
        return false;
    }
    
    memcpy(features->page1, resp->data + 3, sizeof(features->page1));
    XYLog("Read debug features done\n");
    
    return true;
}

bool BtIntel::
setDebugFeatures(IntelDebugFeatures *features)
{
    uint8_t buf[CMD_BUF_MAX_SIZE];
    HciCommandHdr *cmd = (HciCommandHdr *)buf;
    uint8_t mask[11] = { 0x0a, 0x92, 0x02, 0x07, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00 };
    
    if (!features) {
        XYLog("No debug features!\n");
        return false;
    }
    
    if (!(features->page1[0] & 0x3f)) {
        XYLog("Telemetry exception format not supported\n");
        return false;
    }
    
    memset(buf, 0, sizeof(buf));
    cmd->opcode = OSSwapHostToLittleInt16(0xfc8b);
    cmd->len = 11;
    memcpy(cmd->data, mask, 11);
    
    if (!intelSendHCISync(cmd, NULL, 0, NULL, HCI_INIT_TIMEOUT)) {
        XYLog("Setting Intel telemetry ddc write event mask failed\n");
        return false;
    }
    
    XYLog("Set debug features done\n");
    
    return true;
}
