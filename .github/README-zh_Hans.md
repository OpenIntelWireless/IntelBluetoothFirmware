# IntelBluetoothFirmware

- [English](/README.md)
- **简体中文**

IntelBluetoothFirmware 是一个用于在 macOS 中启用原生蓝牙的固件上传驱动，固件的二进制文件来自 Linux。

经过数月的测试后，这个驱动已经被证实可以正常稳定工作。目前支持 macOS 10.13 及以上，支持的设备 ID 如下：

- 0x8087, 0x0a2a
- 0x8087, 0x07dc
- 0x8087, 0x0aa7
- 0x8087, 0x0025
- 0x8087, 0x0aaa
- 0x8087, 0x0026
- 0x8087, 0x0029
- 0x8087, 0x0a2b

## 安装

下载[最新 release](https://github.com/zxystd/IntelBluetoothFirmware/releases/latest)，注入到引导工具后重启即可。

或者解压后,置于`/EFI/EFI/CLOVER/kexts/Other`,跟其他`*.kext`文件放在一起.然后重建缓存`sudo kextcache -i /`,重启.

亲测小米pro gtx 双系统无需再热启动,并且可以关闭蓝牙.

**不要** 把驱动安装到 `/Library/Extensions` 或 `/System/Library/Extensions`，系统很有可能因此冻结。

- **IntelBluetoothFirmware.kext**
  > 用于上传固件
- **IntelBluetoothInjector.kext**
  > 用于修复系统设置内的蓝牙开关

## 排错

如果驱动程序有问题，请在终端中运行以下命令：

```sh
log show --last boot | grep IntelFirmware
```

保存驱动程序日志，并在 issue 中附上。 **如果没有日志生成，你需要先检查引导工具，USB，BIOS 等等。**
