IntelBluetoothFirmware
=========
IntelBluetoothFirmware is a driver that can upload intel wireless bluetooth firmware to support native bluetooth. The firmware binary files are com from linux open source project.

After a few months public test, it seems like it is work well and stable. Currently it is only support 10.12 or greater version system, support device ids as below:

- 0x8087, 0x0a2a
- 0x8087, 0x07dc
- 0x8087, 0x0aa7
- 0x8087, 0x0025
- 0x8087, 0x0aaa
- 0x8087, 0x0026
- 0x8087, 0x0029
- 0x8087, 0x0a2b

Installation
-------
Download the two files from the latest release, and put then in to the kext directory and then restart.
**Do not** put the kext files to L/E or S/L/E, may be it will cause system freeze.
- **IntelBluetoothFirmware.kext**  driver to upload firmware.
- **IntelBluetoothInjector.kext** it is just only provide and display open/close switch on the bluetooth panel, it is not nesseary to install.

Troublesshooting
-------
In case there is something wrong cause the drivers not work, please use terminal and run command
log show --last boot | grep IntelFirmware
to take the driver logs, send it to me or raise issue. If there is no log, maybe you should check USB、DSDT、bios etc.
