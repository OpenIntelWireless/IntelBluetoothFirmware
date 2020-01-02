# IntelBluetoothFirmware

Intel 蓝牙固件上传驱动

**Languages:**

- **简体中文 (Chinese)**
- [English](.github/README-En.md)

## 支持列表

|        型号        | Product ID |
| :----------------: | :--------: |
|      **3165**      |  `0x0a2a`  |
|      **3168**      |  `0x0aa7`  |
|      **7260**      |  `0x07dc`  |
| **8265<br/> 8275** |  `0x0a2b`  |
|     **9260AC**     |  `0x0025`  |
|     **9560AC**     |  `0x0aaa`  |
|      **???**       |  `0x0026`  |
|     **AX200**      |  `0x0029`  |

## 下载

[![Release](https://img.shields.io/github/v/release/zxystd/IntelBluetoothFirmware)](https://github.com/zxystd/IntelBluetoothFirmware/releases/latest)

- IntelBluetoothFirmware 用于上传固件
- IntelBluetoothInjector 用于修复系统设置内的蓝牙开关

## 使用

将驱动正确添加进引导工具中，重启

### 关于测试

欢迎测试和提 issue。测试成功的请留下 `机型`-`系统版本`-`网卡型号`-`pid:vid`，谢谢。

> pid 为 product ID
>
> vid 为 vendor ID (Intel 是 0x8087)
