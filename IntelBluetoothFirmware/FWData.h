//
//  FWData.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/12/22.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef FWData_h
#define FWData_h

#include "ibt-11-5.h"
#include "ibt-12-16.h"
#include "ibt-17-0-1.h"
#include "ibt-17-1.h"
#include "ibt-17-2.h"
#include "ibt-17-16-1.h"
#include "ibt-18-0-1.h"
#include "ibt-18-1.h"
#include "ibt-18-2.h"
#include "ibt-18-16-1.h"
#include "ibt-19-0-0.h"
#include "ibt-19-0-1.h"
#include "ibt-19-0-4.h"
#include "ibt-19-16-4.h"
#include "ibt-19-32-0.h"
#include "ibt-19-32-1.h"
#include "ibt-19-32-4.h"
#include "ibt-20-0-3.h"
#include "ibt-20-1-3.h"
#include "ibt-20-1-4.h"
#include "ibt-hw-37.7.10-fw-1.0.1.2d.d.h"
#include "ibt-hw-37.7.10-fw-1.0.2.3.d.h"
#include "ibt-hw-37.7.10-fw-1.80.1.2d.d.h"
#include "ibt-hw-37.7.10-fw-1.80.2.3.d.h"
#include "ibt-hw-37.7.h"
#include "ibt-hw-37.8.10-fw-1.10.2.27.d.h"
#include "ibt-hw-37.8.10-fw-1.10.3.11.e.h"
#include "ibt-hw-37.8.10-fw-22.50.19.14.f.h"
#include "ibt-hw-37.8.h"

struct FwDesc {
    const char *name;
    const unsigned char *var;
    const long int size;
};

#define IBT_FW(fw_name, fw_var, fw_size) \
    .name = fw_name, .var = fw_var, .size = fw_size

static const struct FwDesc fwList[] = {
    {IBT_FW("ibt-11-5", ibt_11_5_sfi, ibt_11_5_sfi_size)},
    {IBT_FW("ibt-12-16", ibt_12_16_sfi, ibt_12_16_sfi_size)},
    {IBT_FW("ibt-17-0-1", ibt_17_0_1_sfi, ibt_17_0_1_sfi_size)},
    {IBT_FW("ibt-17-1", ibt_17_1_sfi, ibt_17_1_sfi_size)},
    {IBT_FW("ibt-17-2", ibt_17_2_sfi, ibt_17_2_sfi_size)},
    {IBT_FW("ibt-17-16-1", ibt_17_16_1_sfi, ibt_17_16_1_sfi_size)},
    {IBT_FW("ibt-18-0-1", ibt_18_0_1_sfi, ibt_18_0_1_sfi_size)},
    {IBT_FW("ibt-18-1", ibt_18_1_sfi, ibt_18_1_sfi_size)},
    {IBT_FW("ibt-18-2", ibt_18_2_sfi, ibt_18_2_sfi_size)},
    {IBT_FW("ibt-18-16-1", ibt_18_16_1_sfi, ibt_18_16_1_sfi_size)},
    {IBT_FW("ibt-19-0-0", ibt_19_0_0_sfi, ibt_19_0_0_sfi_size)},
    {IBT_FW("ibt-19-0-1", ibt_19_0_1_sfi, ibt_19_0_1_sfi_size)},
    {IBT_FW("ibt-19-0-4", ibt_19_0_4_sfi, ibt_19_0_4_sfi_size)},
    {IBT_FW("ibt-19-16-4", ibt_19_16_4_sfi, ibt_19_16_4_sfi_size)},
    {IBT_FW("ibt-19-32-0", ibt_19_32_0_sfi, ibt_19_32_0_sfi_size)},
    {IBT_FW("ibt-19-32-1", ibt_19_32_1_sfi, ibt_19_32_1_sfi_size)},
    {IBT_FW("ibt-19-32-4", ibt_19_32_4_sfi, ibt_19_32_4_sfi_size)},
    {IBT_FW("ibt-20-0-3", ibt_20_0_3_sfi, ibt_20_0_3_sfi_size)},
    {IBT_FW("ibt-20-1-3", ibt_20_1_3_sfi, ibt_20_1_3_sfi_size)},
    {IBT_FW("ibt-20-1-4", ibt_20_1_4_sfi, ibt_20_1_4_sfi_size)},
    {IBT_FW("ibt-hw-37.7.10-fw-1.0.1.2d.d", ibt_hw_37_7_10_fw_1_0_1_2d_d_bseq, ibt_hw_37_7_10_fw_1_0_1_2d_d_bseq_size)},
    {IBT_FW("ibt-hw-37.7.10-fw-1.0.2.3.d", ibt_hw_37_7_10_fw_1_0_2_3_d_bseq, ibt_hw_37_7_10_fw_1_0_2_3_d_bseq_size)},
    {IBT_FW("ibt-hw-37.7.10-fw-1.80.1.2d.d", ibt_hw_37_7_10_fw_1_80_1_2d_d_bseq, ibt_hw_37_7_10_fw_1_80_1_2d_d_bseq_size)},
    {IBT_FW("ibt-hw-37.7.10-fw-1.80.2.3.d", ibt_hw_37_7_10_fw_1_80_2_3_d_bseq, ibt_hw_37_7_10_fw_1_80_2_3_d_bseq_size)},
    {IBT_FW("ibt-hw-37.7", ibt_hw_37_7_bseq, ibt_hw_37_7_bseq_size)},
    {IBT_FW("ibt-hw-37.8.10-fw-1.10.2.27.d", ibt_hw_37_8_10_fw_1_10_2_27_d_bseq, ibt_hw_37_8_10_fw_1_10_2_27_d_bseq_size)},
    {IBT_FW("ibt-hw-37.8.10-fw-1.10.3.11.e", ibt_hw_37_8_10_fw_1_10_3_11_e_bseq, ibt_hw_37_8_10_fw_1_10_3_11_e_bseq_size)},
    {IBT_FW("ibt-hw-37.8.10-fw-22.50.19.14.f", ibt_hw_37_8_10_fw_22_50_19_14_f_bseq, ibt_hw_37_8_10_fw_22_50_19_14_f_bseq_size)},
    {IBT_FW("ibt-hw-37.8", ibt_hw_37_8_bseq, ibt_hw_37_8_bseq_size)},
};

static struct FwDesc getFWDescByName(const char* name) {
    for (int i = 0; i < 29; i++) {
        if (strcmp(fwList[i].name, name) == 0) {
            return fwList[i];
        }
    }
    return fwList[0];
}

#endif /* FWData_h */
