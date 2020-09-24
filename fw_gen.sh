#!/bin/bash

#  fw_gen.sh
#  IntelBluetoothFirmware
#
#  Created by qcwap on 2020/2/26.
#  Copyright © 2020 钟先耀. All rights reserved.

target_file="${PROJECT_DIR}"/IntelBluetoothFirmware/FwBinary.cpp
fw_files="${PROJECT_DIR}/IntelBluetoothFirmware/fw/*.*"

rm -rf "$target_file"

{
    echo "//  IntelBluetoothFirmware\n\n//  Copyright © 2020 钟先耀. All rights reserved."
    echo "#include \"FwData.h\""
} >> "$target_file"

find "${PROJECT_DIR}/IntelBluetoothFirmware/fw" -iname '*.*' | while read -r fw ; do
    fw_file_name="$(basename "${fw}")"
    fw_var_name="${fw_file_name//./_}"
    fw_var_name="${fw_var_name//-/_}"
    {
        echo ""
        echo "const unsigned char ${fw_var_name}[] = {"
        xxd -i <"$fw"
        echo "};"
        echo ""
        echo "const long int ${fw_var_name}_size = sizeof(${fw_var_name});"
    } >> "$target_file"
done

{
    echo ""
    echo "const struct FwDesc fwList[] = {"
} >> "$target_file"

i=0
while read -r fw ; do
    fw_file_name="$(basename "$fw")"
    fw_var_name="${fw_file_name//./_}"
    fw_var_name="${fw_var_name//-/_}"
    echo "{IBT_FW(\"$fw_file_name\", $fw_var_name, ${fw_var_name}_size)}," >> "$target_file"
    ((i++))
done < <(find "${PROJECT_DIR}/IntelBluetoothFirmware/fw" -iname '*.*')

{
    echo "};"
    echo ""
    echo "const int fwNumber = $i;"
} >> "$target_file"

