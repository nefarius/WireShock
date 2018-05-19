/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <usb.h>
#include <usbdlib.h>
#include <wdfusb.h>
#include <initguid.h>

#include "WireShock.h"
#include "ByteArray.h"
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
