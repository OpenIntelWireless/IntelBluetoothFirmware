//
//  BtIntel.cpp
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#include "BtIntel.h"

uint8_t BtIntel::IntelConvertSpeed(unsigned int speed)
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
