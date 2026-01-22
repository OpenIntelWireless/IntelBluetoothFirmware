//
//  IntelBluetoothGen1.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/6/17.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "IntelBluetoothOpsGen1.hpp"
#include "Log.h"
#include "Hci.h"

OSDefineMetaClassAndStructors(IntelBluetoothOpsGen1, BtIntel)

bool IntelBluetoothOpsGen1::
setup()
{
    const uint8_t *fw_ptr;
    bool disablePatch;
    IntelVersion ver;
    OSData *fwData = NULL;
    char fwname[64];
    
    /* The controller has a bug with the first HCI command sent to it
     * returning number of completed commands as zero. This would stall the
     * command processing in the Bluetooth core.
     *
     * As a workaround, send HCI Reset command first which will reset the
     * number of completed commands and allow normal command processing
     * from now on.
     */
    if (!hciReset()) {
        XYLog("sending initial HCI reset command failed\n");
        return false;
    }
    
    /* Read Intel specific controller version first to allow selection of
     * which firmware file to load.
     *
     * The returned information are hardware variant and revision plus
     * firmware variant, revision and build number.
     */
    if (!readVersion(&ver)) {
        XYLog("Read version failed: %d\n", __LINE__);
        return false;
    }
    
    XYLog("read Intel version: %02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
          ver.hw_platform, ver.hw_variant, ver.hw_revision,
          ver.fw_variant,  ver.fw_revision, ver.fw_build_num,
          ver.fw_build_ww, ver.fw_build_yy, ver.fw_patch_num);
    
    /* Opens the firmware patch file based on the firmware version read
     * from the controller. If it fails to open the matching firmware
     * patch file, it tries to open the default firmware patch file.
     * If no patch file is found, allow the device to operate without
     * a patch.
     */
    fwData = getFirmware(&ver, fwname, sizeof(fwname));
    
    strncpy(this->loadedFirmwareName, fwname, sizeof(this->loadedFirmwareName));
    
    /* fw_patch_num indicates the version of patch the device currently
     * have. If there is no patch data in the device, it is always 0x00.
     * So, if it is other than 0x00, no need to patch the device again.
     */
    if (ver.fw_patch_num) {
        XYLog("Intel device is already patched. patch num: %02x\n", ver.fw_patch_num);
        goto complete;
    }
    
    if (!fwData) {
        goto complete;
    }
    
    fw_ptr = (uint8_t *)fwData->getBytesNoCopy();
    
    /* Enable the manufacturer mode of the controller.
     * Only while this mode is enabled, the driver can download the
     * firmware patch data and configuration parameters.
     */
    if (!enterMfg()) {
        XYLog("Enter mfg failed: %d\n", __LINE__);
        OSSafeReleaseNULL(fwData);
        return false;
    }
    
    disablePatch = true;
    
    /* The firmware data file consists of list of Intel specific HCI
     * commands and its expected events. The first byte indicates the
     * type of the message, either HCI command or HCI event.
     *
     * It reads the command and its expected event from the firmware file,
     * and send to the controller. Once __hci_cmd_sync_ev() returns,
     * the returned event is compared with the event read from the firmware
     * file and it will continue until all the messages are downloaded to
     * the controller.
     *
     * Once the firmware patching is completed successfully,
     * the manufacturer mode is disabled with reset and activating the
     * downloaded patch.
     *
     * If the firmware patching fails, the manufacturer mode is
     * disabled with reset and deactivating the patch.
     *
     * If the default patch file is used, no reset is done when disabling
     * the manufacturer.
     */
    while (fwData->getLength() > fw_ptr - (uint8_t *)fwData->getBytesNoCopy()) {
        if (!patching(fwData, &fw_ptr, &disablePatch))
            goto exit_mfg_deactivate;
    }
    
    XYLog("Patch file download done\n");
    
    OSSafeReleaseNULL(fwData);
    
    if (disablePatch)
        goto exit_mfg_disable;
    
    /* Patching completed successfully and disable the manufacturer mode
     * with reset and activate the downloaded firmware patches.
     */
    if (!exitMfg(true, true)) {
        XYLog("Exit mfg failed: %d\n", __LINE__);
        return false;
    }
    
    /* Need build number for downloaded fw patches in
     * every power-on boot
     */
    if (!readVersion(&ver)) {
        XYLog("Read version failed: %d\n", __LINE__);
        return false;
    }
    
    XYLog("Intel BT fw patch 0x%02x completed & activated\n",
          ver.fw_patch_num);
    
    goto complete;
    
exit_mfg_disable:
    /* Disable the manufacturer mode without reset */
    if (!exitMfg(false, false)) {
        XYLog("Exit mfg failed: %d\n", __LINE__);
        return false;
    }
    
    XYLog("Intel firmware patch completed\n");
    goto complete;
    
exit_mfg_deactivate:
    /* Patching failed. Disable the manufacturer mode with reset and
     * deactivate the downloaded firmware patches.
     */
    if (!exitMfg(true, false)) {
        XYLog("Exit mfg failed %d\n", __LINE__);
        return false;
    }
    XYLog("Intel firmware patch completed and deactivated\n");
    
complete:
    OSSafeReleaseNULL(fwData);
    /* Set the event mask for Intel specific vendor events. This enables
     * a few extra events that are useful during general operation.
     */
    setEventMaskMfg(false);
    
    return true;
}

bool IntelBluetoothOpsGen1::
shutdown()
{
    return true;
}

bool IntelBluetoothOpsGen1::
getFirmwareName(char *fwname, size_t len)
{
    strncpy(fwname, this->loadedFirmwareName, len - 1);
    fwname[len - 1] = '\0';
    return true;
}

bool IntelBluetoothOpsGen1::
hciReset()
{
    HciCommandHdr cmd = {
        .opcode = OSSwapHostToLittleInt16(HCI_OP_RESET),
        .len = 0,
    };
    
    return intelSendHCISync(&cmd, NULL, 0, NULL, HCI_INIT_TIMEOUT);
}

OSData *IntelBluetoothOpsGen1::
getFirmware(IntelVersion *ver, char *fwname, size_t len)
{
    OSData *fwData;
    
    snprintf(fwname, len,
             "ibt-hw-%x.%x.%x-fw-%x.%x.%x.%x.%x.bseq",
             ver->hw_platform, ver->hw_variant, ver->hw_revision,
             ver->fw_variant,  ver->fw_revision, ver->fw_build_num,
             ver->fw_build_ww, ver->fw_build_yy);
    fwData = requestFirmwareData(fwname);
    if (!fwData) {
        XYLog("failed to open Intel firmware file: %s\n", fwname);
        /* If the correct firmware patch file is not found, use the
         * default firmware patch file instead
         */
        snprintf(fwname, len, "ibt-hw-%x.%x.bseq",
                 ver->hw_platform, ver->hw_variant);
        fwData = requestFirmwareData(fwname);
    }
    XYLog("Intel Bluetooth firmware file: %s\n", fwname);
    return fwData;
}

bool IntelBluetoothOpsGen1::
patching(OSData *fwData, const uint8_t **fw_ptr, bool *disablePatch)
{
    const uint8_t *cmdParam;
    FWCommandHdr *cmd = NULL;
    const uint8_t *evtParam = NULL;
    HciEventHdr *evt = NULL;
    uint8_t sendBuf[CMD_BUF_MAX_SIZE];
    uint8_t respBuf[CMD_BUF_MAX_SIZE];
    HciCommandHdr *hciCmd = (HciCommandHdr *)sendBuf;
    HciResponse *resp = (HciResponse *)respBuf;
    uint32_t actRespLen = 0;
    int remain = (int)(fwData->getLength() - (*fw_ptr - (uint8_t *)fwData->getBytesNoCopy()));
    
    /* The first byte indicates the types of the patch command or event.
     * 0x01 means HCI command and 0x02 is HCI event. If the first bytes
     * in the current firmware buffer doesn't start with 0x01 or
     * the size of remain buffer is smaller than HCI command header,
     * the firmware file is corrupted and it should stop the patching
     * process.
     */
    if (remain < HCI_COMMAND_HDR_SIZE || *fw_ptr[0] != 0x01) {
        XYLog("Intel fw corrupted: invalid cmd read\n");
        return false;
    }
    (*fw_ptr)++;
    remain--;
    
    cmd = (FWCommandHdr *)(*fw_ptr);
    *fw_ptr += sizeof(*cmd);
    remain -= sizeof(*cmd);
    
    /* Ensure that the remain firmware data is long enough than the length
     * of command parameter. If not, the firmware file is corrupted.
     */
    if (remain < cmd->len) {
        XYLog("Intel fw corrupted: invalid cmd len\n");
        return false;
    }
    /* If there is a command that loads a patch in the firmware
     * file, then enable the patch upon success, otherwise just
     * disable the manufacturer mode, for example patch activation
     * is not required when the default firmware patch file is used
     * because there are no patch data to load.
     */
    if (*disablePatch && OSSwapLittleToHostInt16(cmd->opcode) == 0xfc8e)
        *disablePatch = false;
    
    cmdParam = *fw_ptr;
    *fw_ptr += cmd->len;
    remain -= cmd->len;
    
    /* This reads the expected events when the above command is sent to the
     * device. Some vendor commands expects more than one events, for
     * example command status event followed by vendor specific event.
     * For this case, it only keeps the last expected event. so the command
     * can be sent with __hci_cmd_sync_ev() which returns the sk_buff of
     * last expected event.
     */
    while (remain > HCI_EVENT_HDR_SIZE && *fw_ptr[0] == 0x02) {
        (*fw_ptr)++;
        remain--;
        
        evt = (HciEventHdr *)(*fw_ptr);
        *fw_ptr += sizeof(*evt);
        remain -= sizeof(*evt);
        
        if (remain < evt->len) {
            XYLog("Intel fw corrupted: invalid evt len\n");
            return false;
        }
        
        evtParam = *fw_ptr;
        *fw_ptr += evt->len;
        remain -= evt->len;
    }
    
    /* Every HCI commands in the firmware file has its correspond event.
     * If event is not found or remain is smaller than zero, the firmware
     * file is corrupted.
     */
    if (!evt || !evtParam || remain < 0) {
        XYLog("Intel fw corrupted: invalid evt read\n");
        return false;
    }
    
    memset(sendBuf, 0, sizeof(sendBuf));
    memset(respBuf, 0, sizeof(respBuf));
    hciCmd->opcode = OSSwapLittleToHostInt16(cmd->opcode);
    hciCmd->len = cmd->len;
    memcpy(hciCmd->data, cmdParam, hciCmd->len);
    
    if (!intelSendHCISyncEvent(hciCmd, resp, sizeof(respBuf), &actRespLen, evt->evt, HCI_INIT_TIMEOUT)) {
        XYLog("sending Intel patch command (0x%4.4x) failed\n", hciCmd->opcode);
        return false;
    }
    
    return true;
}
