# IntelBluetoothFirmware

Intel Bluetooth Firmware Uploader Kernel Extension

**Languages:**

- [简体中文 (Chinese)](/README.md)
- **English**

## Supported Devices

|        Model        | Product ID |
| :----------------: | :--------: |
|      **3165**      |  `0x0a2a`  |
|      **3168**      |  `0x0aa7`  |
|      **7260**      |  `0x07dc`  |
| **8265<br/> 8275** |  `0x0a2b`  |
|     **9260AC**     |  `0x0025`  |
|     **9560AC**     |  `0x0aaa`  |
|      **???**       |  `0x0026`  |
|     **AX200**      |  `0x0029`  |

## Download

[![Release](https://img.shields.io/github/v/release/zxystd/IntelBluetoothFirmware)](https://github.com/zxystd/IntelBluetoothFirmware/releases/latest)

- IntelBluetoothFirmware is used to upload firmware
- IntelBluetoothInjector is used to unlock on/off for bluetooth in system preferences

## Usage

Install the Kext in bootloaders properly, then restart

### For Testing

Welcome to test this Kext and submit issues. If you successfull made your Bluetooth working, please provide your `Model`-`OS version`-`pid:vid`

> pid: product ID
>
> vid: vendor ID (Intel: 0x8087)
