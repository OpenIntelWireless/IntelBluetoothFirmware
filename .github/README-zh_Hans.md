# IntelBluetoothFirmware

![CI](https://github.com/OpenIntelWireless/IntelBluetoothFirmware/workflows/CI/badge.svg)

- [English](/README.md)
- **简体中文**

> [!IMPORTANT]
> 本分支中的 **LE Data Length Change 修复**——用于清除 bluetoothd 在 macOS Tahoe 上卡在 `LE_READY_WAIT_FOR_LENGTH` 的问题，使 BLE HID 设备得以绑定（[#514](https://github.com/OpenIntelWireless/IntelBluetoothFirmware/issues/514)）——由 **AI 助手 Claude 编写**。该修复按原样提供，不含任何担保；您需自行负责审查、测试，并自担风险使用。

## 简介

IntelBluetoothFirmware 是一个用于在 macOS 中启用原生蓝牙的固件上传驱动，固件的二进制文件来自 Linux。

经过数月的测试后，这个驱动已经被证实可以正常稳定工作。

[![Join the chat at https://gitter.im/OpenIntelWireless/itlwm](https://badges.gitter.im/OpenIntelWireless/IntelBluetoothFirmware.svg)](https://gitter.im/OpenIntelWireless/IntelBluetoothFirmware?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## 文档

**请在使用驱动或提交 issue 之前仔细阅读文档！**

- [支持列表](https://openintelwireless.github.io/IntelBluetoothFirmware/Compat.html)
- [安装方法](https://openintelwireless.github.io/IntelBluetoothFirmware/Installation.html)
- [常见疑问](https://openintelwireless.github.io/IntelBluetoothFirmware/FAQ.html)
- [排错方法](https://openintelwireless.github.io/IntelBluetoothFirmware/Troubleshooting.html)

## 参考资料

- [torvalds/linux](https://github.com/torvalds/linux)
- [acidanthera/BrcmPatchRAM](https://github.com/acidanthera/BrcmPatchRAM)
