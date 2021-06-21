/** @file
  Copyright (c) 2020 zxystd. All rights reserved.
  SPDX-License-Identifier: GPL-3.0-only
**/

//
//  Hci.h
//  IntelBluetoothFirmware
//
//  Created by zxystd on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef Hci_h
#define Hci_h

typedef struct __attribute__((packed))
{
    uint16_t    opcode;    /* OCF & OGF */
    uint8_t     len;
    uint8_t     data[];
} HciCommandHdr;

typedef struct __attribute__((packed))
{
    uint8_t     evt;
    uint8_t     len;
} HciEventHdr;

typedef struct __attribute__((packed)) 
{
    HciEventHdr evt;
    uint8_t     numCommands;
    uint16_t    opcode;
    uint8_t     data[];
} HciResponse;

typedef struct __attribute__((packed))
{
    uint8_t     status;
    uint8_t     numCommands;
    uint16_t    opcode;
} HciCmdStatus;

/* ---- HCI Events ---- */
#define HCI_EV_INQUIRY_COMPLETE                 0x01
#define HCI_EV_INQUIRY_RESULT                   0x02
#define HCI_EV_CONN_COMPLETE                    0x03
#define HCI_EV_CONN_REQUEST                     0x04
#define HCI_EV_DISCONN_COMPLETE                 0x05
#define HCI_EV_AUTH_COMPLETE                    0x06
#define HCI_EV_REMOTE_NAME                      0x07
#define HCI_EV_ENCRYPT_CHANGE                   0x08
#define HCI_EV_CHANGE_LINK_KEY_COMPLETE         0x09
#define HCI_EV_REMOTE_FEATURES                  0x0b
#define HCI_EV_REMOTE_VERSION                   0x0c
#define HCI_EV_QOS_SETUP_COMPLETE               0x0d
#define HCI_EV_CMD_COMPLETE                     0x0e
#define HCI_EV_CMD_STATUS                       0x0f
#define HCI_EV_HARDWARE_ERROR                   0x10
#define HCI_EV_ROLE_CHANGE                      0x12
#define HCI_EV_NUM_COMP_PKTS                    0x13
#define HCI_EV_MODE_CHANGE                      0x14
#define HCI_EV_PIN_CODE_REQ                     0x16
#define HCI_EV_LINK_KEY_REQ                     0x17
#define HCI_EV_LINK_KEY_NOTIFY                  0x18
#define HCI_EV_CLOCK_OFFSET                     0x1c
#define HCI_EV_PKT_TYPE_CHANGE                  0x1d
#define HCI_EV_PSCAN_REP_MODE                   0x20
#define HCI_EV_INQUIRY_RESULT_WITH_RSSI         0x22
#define HCI_EV_REMOTE_EXT_FEATURES              0x23
#define HCI_EV_SYNC_CONN_COMPLETE               0x2c
#define HCI_EV_SYNC_CONN_CHANGED                0x2d
#define HCI_EV_SNIFF_SUBRATE                    0x2e
#define HCI_EV_EXTENDED_INQUIRY_RESULT          0x2f
#define HCI_EV_KEY_REFRESH_COMPLETE             0x30
#define HCI_EV_IO_CAPA_REQUEST                  0x31
#define HCI_EV_IO_CAPA_REPLY                    0x32
#define HCI_EV_USER_CONFIRM_REQUEST             0x33
#define HCI_EV_USER_PASSKEY_REQUEST             0x34
#define HCI_EV_REMOTE_OOB_DATA_REQUEST          0x35
#define HCI_EV_SIMPLE_PAIR_COMPLETE             0x36
#define HCI_EV_USER_PASSKEY_NOTIFY              0x3b
#define HCI_EV_KEYPRESS_NOTIFY                  0x3c
#define HCI_EV_REMOTE_HOST_FEATURES             0x3d
#define HCI_EV_LE_META                          0x3e
#define HCI_EV_PHY_LINK_COMPLETE                0x40
#define HCI_EV_CHANNEL_SELECTED                 0x41
#define HCI_EV_DISCONN_PHY_LINK_COMPLETE        0x42
#define HCI_EV_LOGICAL_LINK_COMPLETE            0x45
#define HCI_EV_DISCONN_LOGICAL_LINK_COMPLETE    0x46
#define HCI_EV_NUM_COMP_BLOCKS                  0x48
#define HCI_EV_SYNC_TRAIN_COMPLETE              0x4F
#define HCI_EV_SLAVE_PAGE_RESP_TIMEOUT          0x54

/* HCI timeouts */
#define HCI_DISCONN_TIMEOUT     2000    /*  2 seconds */
#define HCI_PAIRING_TIMEOUT     60000   /* 60 seconds */
#define HCI_INIT_TIMEOUT        10000   /* 10 seconds */
#define HCI_CMD_TIMEOUT         2000    /*  2 seconds */
#define HCI_ACL_TX_TIMEOUT      45000   /* 45 seconds */
#define HCI_AUTO_OFF_TIMEOUT    2000    /*  2 seconds */
#define HCI_POWER_OFF_TIMEOUT   5000    /*  5 seconds */
#define HCI_LE_CONN_TIMEOUT     20000   /* 20 seconds */
#define HCI_LE_AUTOCONN_TIMEOUT 4000    /*  4 seconds */

/* HCI data types */
#define HCI_COMMAND_PKT 0x01
#define HCI_ACLDATA_PKT 0x02
#define HCI_SCODATA_PKT 0x03
#define HCI_EVENT_PKT   0x04
#define HCI_DIAG_PKT    0xf0
#define HCI_VENDOR_PKT  0xff

/* -----  HCI Commands ---- */
#define HCI_OP_NOP                          0x0000
#define HCI_OP_INQUIRY                      0x0401
#define HCI_OP_INQUIRY_CANCEL               0x0402
#define HCI_OP_PERIODIC_INQ                 0x0403
#define HCI_OP_EXIT_PERIODIC_INQ            0x0404
#define HCI_OP_CREATE_CONN                  0x0405
#define HCI_OP_DISCONNECT                   0x040
#define HCI_OP_ADD_SCO                      0x0407
#define HCI_OP_CREATE_CONN_CANCEL           0x0408
#define HCI_OP_ACCEPT_CONN_REQ              0x0409
#define HCI_OP_REJECT_CONN_REQ              0x040a
#define HCI_OP_LINK_KEY_REPLY               0x040b
#define HCI_OP_LINK_KEY_NEG_REPLY           0x040c
#define HCI_OP_PIN_CODE_REPLY               0x040d
#define HCI_OP_PIN_CODE_NEG_REPLY           0x040e
#define HCI_OP_CHANGE_CONN_PTYPE            0x040f
#define HCI_OP_AUTH_REQUESTED               0x0411
#define HCI_OP_SET_CONN_ENCRYPT             0x0413
#define HCI_OP_CHANGE_CONN_LINK_KEY         0x0415
#define HCI_OP_REMOTE_NAME_REQ              0x0419
#define HCI_OP_REMOTE_NAME_REQ_CANCEL       0x041a
#define HCI_OP_READ_REMOTE_FEATURES         0x041b
#define HCI_OP_READ_REMOTE_EXT_FEATURES     0x041c
#define HCI_OP_READ_REMOTE_VERSION          0x041d
#define HCI_OP_READ_CLOCK_OFFSET            0x041f
#define HCI_OP_SETUP_SYNC_CONN              0x0428
#define HCI_OP_ACCEPT_SYNC_CONN_REQ         0x0429
#define HCI_OP_REJECT_SYNC_CONN_REQ         0x042a
#define HCI_OP_IO_CAPABILITY_REPLY          0x042b
#define HCI_OP_USER_CONFIRM_REPLY           0x042c
#define HCI_OP_USER_CONFIRM_NEG_REPLY       0x042d
#define HCI_OP_USER_PASSKEY_REPLY           0x042e
#define HCI_OP_USER_PASSKEY_NEG_REPLY       0x042f
#define HCI_OP_REMOTE_OOB_DATA_REPLY        0x0430
#define HCI_OP_REMOTE_OOB_DATA_NEG_REPLY    0x0433
#define HCI_OP_IO_CAPABILITY_NEG_REPLY      0x0434
#define HCI_OP_CREATE_PHY_LINK              0x0435
#define HCI_OP_ACCEPT_PHY_LINK              0x0436
#define HCI_OP_DISCONN_PHY_LINK             0x0437
#define HCI_OP_CREATE_LOGICAL_LINK          0x0438
#define HCI_OP_ACCEPT_LOGICAL_LINK          0x0439
#define HCI_OP_DISCONN_LOGICAL_LINK         0x043a
#define HCI_OP_LOGICAL_LINK_CANCEL          0x043b
#define HCI_OP_SET_CSB                      0x0441
#define HCI_OP_START_SYNC_TRAIN             0x0443
#define HCI_OP_REMOTE_OOB_EXT_DATA_REPLY    0x0445
#define HCI_OP_SNIFF_MODE                   0x0803
#define HCI_OP_EXIT_SNIFF_MODE              0x0804
#define HCI_OP_ROLE_DISCOVERY               0x0809
#define HCI_OP_SWITCH_ROLE                  0x080b
#define HCI_OP_READ_LINK_POLICY             0x080c
#define HCI_OP_WRITE_LINK_POLICY            0x080d
#define HCI_OP_READ_DEF_LINK_POLICY         0x080e
#define HCI_OP_WRITE_DEF_LINK_POLICY        0x080f
#define HCI_OP_SNIFF_SUBRATE                0x0811
#define HCI_OP_SET_EVENT_MASK               0x0c01
#define HCI_OP_RESET                        0x0c03
#define HCI_OP_SET_EVENT_FLT                0x0c05

/* Command opcode pack/unpack */
#define hci_opcode_pack(ogf, ocf)    ((__u16) ((ocf & 0x03ff)|(ogf << 10)))
#define hci_opcode_ogf(op)        (op >> 10)
#define hci_opcode_ocf(op)        (op & 0x03ff)

/* ---- HCI Packet structures ---- */
#define HCI_COMMAND_HDR_SIZE 3
#define HCI_EVENT_HDR_SIZE   2
#define HCI_ACL_HDR_SIZE     4
#define HCI_SCO_HDR_SIZE     3

#endif /* Hci_h */
