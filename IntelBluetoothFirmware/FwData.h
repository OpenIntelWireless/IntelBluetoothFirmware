/** @file
  Copyright (c) 2020 zxystd. All rights reserved.
  SPDX-License-Identifier: GPL-3.0-only
**/

//
//  FwData.h
//  IntelBluetoothFirmware
//
//  Created by zxystd on 2019/12/22.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef FwData_h
#define FwData_h
#include <string.h>
#include <libkern/c++/OSData.h>
#include <libkern/zlib.h>
#include <zutil.h>

struct FwDesc {
    const char *name;
    const unsigned char *var;
    const long int size;
};

#define IBT_FW(fw_name, fw_var, fw_size) \
    .name = fw_name, .var = fw_var, .size = fw_size


extern const struct FwDesc fwList[];
extern const int fwNumber;

static inline OSData *getFWDescByName(const char* name) {
    for (int i = 0; i < fwNumber; i++) {
        if (strcmp(fwList[i].name, name) == 0) {
            FwDesc desc = fwList[i];
            return OSData::withBytes(desc.var, (unsigned int)desc.size);
        }
    }
    return NULL;
}

static inline bool uncompressFirmware(unsigned char *dest, uint *destLen, unsigned char *source, uint sourceLen)
{
    z_stream stream;
    int err;
    
    stream.next_in = source;
    stream.avail_in = sourceLen;
    stream.next_out = dest;
    stream.avail_out = *destLen;
    stream.zalloc = zcalloc;
    stream.zfree = zcfree;
    err = inflateInit(&stream);
    if (err != Z_OK) {
        return false;
    }
    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        return false;
    }
    *destLen = (uint)stream.total_out;

    err = inflateEnd(&stream);
    return err == Z_OK;
}

#endif /* FwData_h */
