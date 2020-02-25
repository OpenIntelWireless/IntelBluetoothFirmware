# IntelBluetoothFirmware

- **English**
- [简体中文](/.github/README-zh_Hans.md)

IntelBluetoothFirmware is a Kernel Extension that uploads Intel Wireless Bluetooth Firmware to provide native Bluetooth in macOS.
The firmware binary files are com from the Linux Open Source Project.

After a few months of public testing, it seems like this Kext works well and stable.
Currently it supports macOS 10.13 or higher, supported device ids are:

- 0x8087, 0x0a2a
- 0x8087, 0x07dc
- 0x8087, 0x0aa7
- 0x8087, 0x0025
- 0x8087, 0x0aaa
- 0x8087, 0x0026
- 0x8087, 0x0029
- 0x8087, 0x0a2b

## Installation

Download the [latest release](https://github.com/zxystd/IntelBluetoothFirmware/releases/latest), inject the Kext files into the Bootloader and then restart.

***Do not*** inject the Kext files to `/Library/Extensions` or `/System/Library/Extensions` as it may likely **freeze the system**.

- **IntelBluetoothFirmware.kext**
  > Driver to upload the firmware.
- **IntelBluetoothInjector.kext**
  > Dummy Kext to enable open/close switch on the Bluetooth settings panel, not necessary to install.

## Troubleshooting

In case there is something wrong with the driver, please run the following command in Terminal:

```sh
log show --last boot | grep IntelFirmware
```

Save the driver logs, send it to me by opening an issue. **If there are no logs, you should probably check your Bootloader, USB, BIOS, etc.**
