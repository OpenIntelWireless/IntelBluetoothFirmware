//
//  Log.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/12/3.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef Log_h
#define Log_h

#include <IOKit/IOLib.h>

#define XYLog(fmt, x...)\
do\
{\
IOLog("%s: " fmt, "IntelFirmware", ##x);\
}while(0)

#endif /* Log_h */
