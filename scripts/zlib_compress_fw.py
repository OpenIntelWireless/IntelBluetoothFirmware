#!/usr/bin/python3
# -*- coding: utf-8 -*-
# zlib_compress_fw.py
#
#  Created by qcwap on 2020/9/3.
#  Copyright © 2020 钟先耀. All rights reserved.

import zlib
import os
import struct
import hashlib

copyright = '''
//  itlwm
//
//  Copyright © 2020 钟先耀. All rights reserved.
#include "FwData.h"
'''

def compress(data):
    return zlib.compress(data)

def hash(data):
    sha1sum = hashlib.sha1()
    sha1sum.update(data)
    return sha1sum.hexdigest()
    
def format_file_name(file_name):
    return file_name.replace(".", "_").replace("-", "_")

def format_var_name(hash):
    return "firmware_" + hash

def write_single_file(target_file, path, file, file_hash):
    src_file = open(path, "rb")
    src_data = src_file.read()
    src_hash = hash(src_data)
    fw_var_name = format_file_name(file)
    data_var_name = format_var_name(src_hash)

    if src_hash not in file_hash:
        file_hash.append(src_hash)
        src_data = compress(src_data)
        src_len = len(src_data)
        target_file.write("\nconst unsigned char ")
        target_file.write(data_var_name)
        target_file.write("[] = {")
        index = 0;
        block = []
        while True:
            if index + 16 >= src_len:
                block = src_data[index:]
            else:
                block = src_data[index:index + 16]
            index += 16;
            if len(block) < 16:
                if len(block):
                    for b in block:
                        if type(b) is str:
                            b = ord(b)
                        target_file.write("0x{:02X}, ".format(b))
                    target_file.write("\n")
                break
            target_file.write("0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                    "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                    "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                    "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X},\n"
                    .format(*struct.unpack("BBBBBBBBBBBBBBBB", block)))
        target_file.write("};\n")

    target_file.write("\nconst unsigned char *")
    target_file.write(fw_var_name)
    target_file.write(" = ")
    target_file.write(data_var_name)
    target_file.write(";\n")
    target_file.write("const long int ")
    target_file.write(fw_var_name)
    target_file.write("_size = sizeof(")
    target_file.write(data_var_name)
    target_file.write(");\n")
    src_file.close()
    
    
def process_files(target_file, dir):
    if not os.path.exists(target_file):
        if not os.path.exists(os.path.dirname(target_file)):
            os.mkdirs(os.path.dirname(target_file))
    target_file_handle = open(target_file, "w")
    target_file_handle.write(copyright)
    for root, dirs, files in os.walk(dir):
        file_hash=[]
        for file in files:
            path = os.path.join(root, file)
            write_single_file(target_file_handle, path, file, file_hash)
            
        target_file_handle.write("\n")
        target_file_handle.write("const struct FwDesc fwList[] = {")
        
        for file in files:
            target_file_handle.write('{IBT_FW("')
            target_file_handle.write(file)
            target_file_handle.write('", ')
            fw_var_name = format_file_name(file)
            target_file_handle.write(fw_var_name)
            target_file_handle.write(", ")
            target_file_handle.write(fw_var_name)
            target_file_handle.write("_size)},\n")
            
        target_file_handle.write("};\n")
        target_file_handle.write("const int fwNumber = ")
        target_file_handle.write(str(len(files)))
        target_file_handle.write(";\n")
            
    target_file_handle.close()

if __name__ == '__main__':
    print(compress("test"));
