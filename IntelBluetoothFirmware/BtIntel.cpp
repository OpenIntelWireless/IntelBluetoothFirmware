//
//  BtIntel.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#include "BtIntel.h"
#include <IOKit/IOLib.h>
#include <string.h>
#include "Log.h"

uint8_t BtIntel::intelConvertSpeed(unsigned int speed)
{
    switch (speed) {
    case 9600:
        return 0x00;
    case 19200:
        return 0x01;
    case 38400:
        return 0x02;
    case 57600:
        return 0x03;
    case 115200:
        return 0x04;
    case 230400:
        return 0x05;
    case 460800:
        return 0x06;
    case 921600:
        return 0x07;
    case 1843200:
        return 0x08;
    case 3250000:
        return 0x09;
    case 2000000:
        return 0x0a;
    case 3000000:
        return 0x0b;
    default:
        return 0xff;
    }
}

void BtIntel::printIntelVersion(IntelVersion *ver)
{
    const char *variant;

    switch (ver->fw_variant) {
    case 0x06:
        variant = "Bootloader";
        break;
    case 0x23:
        variant = "Firmware";
        break;
    default:
        variant = "Unknown";
        break;
    }
    XYLog("%s revision %u.%u build %u week %u %u",
    variant, ver->fw_revision >> 4, ver->fw_revision & 0x0f,
    ver->fw_build_num, ver->fw_build_ww,
          2000 + ver->fw_build_yy);
}


void BtIntel::printAllByte(void *addr, int size)
{
    unsigned char *ptr = (unsigned char*)addr;
    int print_bytes = 0;
 
    if(NULL == ptr) {
        return;
    }
    
    while(print_bytes < size) {
        IOLog("%02x", *ptr);
        ptr++;
        print_bytes++;
    }
}
