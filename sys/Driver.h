/*
MIT License

Copyright (c) 2018 Benjamin "Nefarius" Höglinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <ntddk.h>
#include <wdf.h>
#include <usb.h>
#include <usbdlib.h>
#include <wdfusb.h>
#include <initguid.h>

#include <DsCommon.h>
#include <km/DsDriver.h>
#include <UsbInterrupt.h>
#include "WireShock.h"
#include "WireBus.h"
#include "Public.h"
#include "device.h"
#include "queue.h"
#include "trace.h"
#include "Bluetooth.h"
#include "HCI.h"
#include "Interrupt.h"
#include "Bulkrwr.h"
#include "L2CAP.h"
#include "Ds3.h"


EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD WireShockEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP WireShockEvtDriverContextCleanup;

EXTERN_C_END
