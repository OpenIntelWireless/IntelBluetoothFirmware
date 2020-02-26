//
//  FWData.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/12/22.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef FWData_h
#define FWData_h
#include <string.h>

struct FwDesc {
    const char *name;
    const unsigned char *var;
    const long int size;
};

#define IBT_FW(fw_name, fw_var, fw_size) \
    .name = fw_name, .var = fw_var, .size = fw_size


extern const struct FwDesc fwList[];
extern const int fwNumber;

static inline struct FwDesc getFWDescByName(const char* name) {
    for (int i = 0; i < fwNumber; i++) {
        if (strcmp(fwList[i].name, name) == 0) {
            return fwList[i];
        }
    }
    return fwList[0];
}

#endif /* FWData_h */
