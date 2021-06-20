//
//  linux.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2021/5/21.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef linux_h
#define linux_h

#include <IOKit/IOTypes.h>
#include <libkern/OSAtomic.h>

typedef UInt8  u8;
typedef UInt16 u16;
typedef UInt32 u32;
typedef UInt64 u64;

typedef u8 __u8;
typedef u16 __u16;
typedef u32 __u32;
typedef u64 __u64;

typedef __u16 __le16;
typedef __u32 __le32;
typedef __u64 __le64;

typedef SInt8  s8;
typedef SInt16 s16;
typedef SInt32 s32;
typedef SInt64 s64;

typedef s8  __s8;
typedef s16 __s16;
typedef s32 __s32;
typedef s64 __s64;

#define __force

static inline __u32 __le32_to_cpup(const __le32 *p)
{
    return (__force __u32)*p;
}

static inline __u16 __le16_to_cpup(const __le16 *p)
{
    return (__force __u16)*p;
}

#define le32_to_cpup __le32_to_cpup
#define le16_to_cpup __le16_to_cpup

static inline u32 get_unaligned_le32(const void *p)
{
    return le32_to_cpup((__le32 *)p);
}

static inline u32 get_unaligned_le16(const void *p)
{
    return le16_to_cpup((__le16 *)p);
}

#endif /* linux_h */
