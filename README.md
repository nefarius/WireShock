# WireShock

Windows Bluetooth Host Driver for Sony DualShock Controllers

---

This was a research project and an attempt of porting over the Bluetooth host stack from the C#-implementation used in [ScpToolkit](https://github.com/nefarius/ScpToolkit) to a native C Windows kernel-mode bus driver. It has been discontinued [in favour of better solutions](https://forums.vigem.org/topic/242/bluetooth-filter-driver-for-ds3-compatibility-research-notes). The code will stay up for anyone to use as either an inspiration or a negative example 😜 Do bear in mind, that the code may contain unaddressed issues! Compile and use at your own risk! No support or binaries provided.

---

## Summary

`WireShock` is a Windows kernel-mode driver implementing a custom Windows Bluetooth Stack handling wireless communication with Sony DualShock 3 and 4 controllers. It acts as a function driver for a variety of USB host radios (see [`WireShock.inf`](sys/WireShock.inf)) and a bus driver to expose connected controllers as HID devices to the system (see [`WireShockHidDevice.inf`](sys/WireShockHidDevice.inf)).

## Architecture

Since the Sony DualShock 3 utilizes a butchered non-standard Bluetooth protocol incompatible with standard HID profiles a custom Bluetooth stack is required to establish a connection on the Windows platform. `WireShock` implements a compatible Bluetooth stack and also acts as a bus emulator allowing for multiple devices to connect and transmit. It's designed to work with most USB Bluetooth host devices obeying at least [Core Version 2.1 + EDR](https://www.bluetooth.com/specifications/bluetooth-core-specification/legacy-specifications) standards.

The actual input and output reports are exposed by HID-compliant child PDOs using a [custom report format](common/src/DsHid.c) to additionally present pressure axes to DirectInput.

## Supported systems

The driver is built for Windows 7/8/8.1/10 (x86 and amd64).

## Sources

- [felis/USB_Host_Shield_2.0](<https://github.com/felis/USB_Host_Shield_2.0>)
- [RetroPie/sixad](<https://github.com/RetroPie/sixad>)
- [nefarius/ScpToolkit](<https://github.com/nefarius/ScpToolkit>)
- [Emulate HID Device with Windows Desktop](<https://nadavrub.wordpress.com/2015/07/17/simulate-hid-device-with-windows-desktop/>)
- [PS3 Information/Bluetooth](<https://github.com/felis/USB_Host_Shield_2.0/wiki/-S3-Information#Bluetooth>)
- [PS3 Developer wiki/DualShock 3](<http://www.psdevwiki.com/ps3/DualShock_3>)
- [Eleccelerator Wiki/DualShock 3](<http://eleccelerator.com/wiki/index.php?title=DualShock_3>)
