#include "Driver.h"
#include "wirebus.tmh"
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <hidport.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WireShockEvtWdfChildListCreateDevice)
#endif

_Use_decl_annotations_
NTSTATUS
WireShockEvtWdfChildListCreateDevice(
    WDFCHILDLIST ChildList,
    PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER IdentificationDescription,
    PWDFDEVICE_INIT ChildInit
)
{
    NTSTATUS        status;

    UNREFERENCED_PARAMETER(ChildList);

    PPDO_IDENTIFICATION_DESCRIPTION pDesc;
    WDF_OBJECT_ATTRIBUTES           pdoAttributes;
    WDFDEVICE                       hChild = NULL;
    WDF_DEVICE_PNP_CAPABILITIES     pnpCaps;
    WDF_DEVICE_POWER_CAPABILITIES   powerCaps;
    WDF_IO_QUEUE_CONFIG             defaultQueueCfg;
    WDFQUEUE                        defaultQueue;

    DECLARE_CONST_UNICODE_STRING(deviceLocation, L"WireShock Bus Device");
    DECLARE_UNICODE_STRING_SIZE(buffer, 1024);
    DECLARE_UNICODE_STRING_SIZE(deviceId, 1024);
    PCWSTR HardwareIds = L"Nefarius\\WireShockHidDevice\0\0";
    ULONG SerialNo = 1;


    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    pDesc = CONTAINING_RECORD(
        IdentificationDescription,
        PDO_IDENTIFICATION_DESCRIPTION,
        Header);

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_WIREBUS,
        "Would plug in device with address %02X:%02X:%02X:%02X:%02X:%02X",
        pDesc->ClientAddress.Address[0],
        pDesc->ClientAddress.Address[1],
        pDesc->ClientAddress.Address[2],
        pDesc->ClientAddress.Address[3],
        pDesc->ClientAddress.Address[4],
        pDesc->ClientAddress.Address[5]);

    WdfDeviceInitSetDeviceType(ChildInit, FILE_DEVICE_BUS_EXTENDER);
    WdfPdoInitAllowForwardingRequestToParent(ChildInit);


    //
    // Provide DeviceID, HardwareIDs, CompatibleIDs and InstanceId
    //
    RtlInitUnicodeString(&deviceId, HardwareIds);

    status = WdfPdoInitAssignDeviceID(ChildInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAssignDeviceID failed with status %!STATUS!",
            status);
        return status;
    }

    //
    // NOTE: same string  is used to initialize hardware id too
    //
    status = WdfPdoInitAddHardwareID(ChildInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddHardwareID failed with status %!STATUS!",
            status);
        return status;
    }

    status = RtlUnicodeStringPrintf(&buffer, L"%02d", SerialNo);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "RtlUnicodeStringPrintf failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoInitAssignInstanceID(ChildInit, &buffer);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAssignInstanceID failed with status %!STATUS!",
            status);
        return status;
    }

    //
    // Provide a description about the device. This text is usually read from
    // the device. In the case of USB device, this text comes from the string
    // descriptor. This text is displayed momentarily by the PnP manager while
    // it's looking for a matching INF. If it finds one, it uses the Device
    // Description from the INF file or the friendly name created by
    // coinstallers to display in the device manager. FriendlyName takes
    // precedence over the DeviceDesc from the INF file.
    //
    status = RtlUnicodeStringPrintf(&buffer,
        L"WireShock HID Device");
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "RtlUnicodeStringPrintf failed with status %!STATUS!",
            status);
        return status;
    }

    //
    // You can call WdfPdoInitAddDeviceText multiple times, adding device
    // text for multiple locales. When the system displays the text, it
    // chooses the text that matches the current locale, if available.
    // Otherwise it will use the string for the default locale.
    // The driver can specify the driver's default locale by calling
    // WdfPdoInitSetDefaultLocale.
    //
    status = WdfPdoInitAddDeviceText(ChildInit,
        &buffer,
        &deviceLocation,
        0x409);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddDeviceText failed with status %!STATUS!",
            status);
        return status;
    }

    WdfPdoInitSetDefaultLocale(ChildInit, 0x409);



    WDF_OBJECT_ATTRIBUTES_INIT(&pdoAttributes);

    status = WdfDeviceCreate(&ChildInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfDeviceCreate failed with status %!STATUS!",
            status);
        return status;
    }

    WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
    pnpCaps.Removable = WdfTrue;
    pnpCaps.EjectSupported = WdfTrue;
    pnpCaps.SurpriseRemovalOK = WdfTrue;

    WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);

    WDF_DEVICE_POWER_CAPABILITIES_INIT(&powerCaps);

    powerCaps.DeviceD1 = WdfTrue;
    powerCaps.WakeFromD1 = WdfTrue;
    powerCaps.DeviceWake = PowerDeviceD1;

    powerCaps.DeviceState[PowerSystemWorking] = PowerDeviceD1;
    powerCaps.DeviceState[PowerSystemSleeping1] = PowerDeviceD1;
    powerCaps.DeviceState[PowerSystemSleeping2] = PowerDeviceD2;
    powerCaps.DeviceState[PowerSystemSleeping3] = PowerDeviceD2;
    powerCaps.DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    powerCaps.DeviceState[PowerSystemShutdown] = PowerDeviceD3;

    WdfDeviceSetPowerCapabilities(hChild, &powerCaps);

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&defaultQueueCfg, WdfIoQueueDispatchParallel);
    defaultQueueCfg.EvtIoInternalDeviceControl = WireChildEvtWdfIoQueueIoInternalDeviceControl;

    status = WdfIoQueueCreate(hChild, &defaultQueueCfg, WDF_NO_OBJECT_ATTRIBUTES, &defaultQueue);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfIoQueueCreate (Default) failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit with status %!STATUS!", status);

    return status;
}

void WireChildEvtWdfIoQueueIoInternalDeviceControl(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    size_t OutputBufferLength,
    size_t InputBufferLength,
    ULONG IoControlCode
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    NTSTATUS            status = STATUS_SUCCESS;
    size_t              bytesToCopy = 0;
    WDFMEMORY           memory;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry (IoControlCode: 0x%X)", IoControlCode);

    typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

    CONST  HID_REPORT_DESCRIPTOR           G_DefaultReportDescriptor[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x09, 0x30,        //   Usage (X)
        0x09, 0x31,        //   Usage (Y)
        0x09, 0x32,        //   Usage (Z)
        0x09, 0x35,        //   Usage (Rz)
        0x15, 0x00,        //   Logical Minimum (0)
        0x26, 0xFF, 0x00,  //   Logical Maximum (255)
        0x75, 0x08,        //   Report Size (8)
        0x95, 0x04,        //   Report Count (4)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x09, 0x39,        //   Usage (Hat switch)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x07,        //   Logical Maximum (7)
        0x35, 0x00,        //   Physical Minimum (0)
        0x46, 0x3B, 0x01,  //   Physical Maximum (315)
        0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
        0x75, 0x04,        //   Report Size (4)
        0x95, 0x01,        //   Report Count (1)
        0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
        0x65, 0x00,        //   Unit (None)
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x0E,        //   Usage Maximum (0x0E)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x0E,        //   Report Count (14)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
        0x09, 0x20,        //   Usage (0x20)
        0x75, 0x06,        //   Report Size (6)
        0x95, 0x01,        //   Report Count (1)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x7F,        //   Logical Maximum (127)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
        0x09, 0x33,        //   Usage (Rx)
        0x09, 0x34,        //   Usage (Ry)
        0x15, 0x00,        //   Logical Minimum (0)
        0x26, 0xFF, 0x00,  //   Logical Maximum (255)
        0x75, 0x08,        //   Report Size (8)
        0x95, 0x02,        //   Report Count (2)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
        0x09, 0x21,        //   Usage (0x21)
        0x95, 0x36,        //   Report Count (54)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x85, 0x05,        //   Report ID (5)
        0x09, 0x22,        //   Usage (0x22)
        0x95, 0x1F,        //   Report Count (31)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x04,        //   Report ID (4)
        0x09, 0x23,        //   Usage (0x23)
        0x95, 0x24,        //   Report Count (36)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x02,        //   Report ID (2)
        0x09, 0x24,        //   Usage (0x24)
        0x95, 0x24,        //   Report Count (36)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x08,        //   Report ID (8)
        0x09, 0x25,        //   Usage (0x25)
        0x95, 0x03,        //   Report Count (3)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x10,        //   Report ID (16)
        0x09, 0x26,        //   Usage (0x26)
        0x95, 0x04,        //   Report Count (4)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x11,        //   Report ID (17)
        0x09, 0x27,        //   Usage (0x27)
        0x95, 0x02,        //   Report Count (2)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x12,        //   Report ID (18)
        0x06, 0x02, 0xFF,  //   Usage Page (Vendor Defined 0xFF02)
        0x09, 0x21,        //   Usage (0x21)
        0x95, 0x0F,        //   Report Count (15)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x13,        //   Report ID (19)
        0x09, 0x22,        //   Usage (0x22)
        0x95, 0x16,        //   Report Count (22)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x14,        //   Report ID (20)
        0x06, 0x05, 0xFF,  //   Usage Page (Vendor Defined 0xFF05)
        0x09, 0x20,        //   Usage (0x20)
        0x95, 0x10,        //   Report Count (16)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x15,        //   Report ID (21)
        0x09, 0x21,        //   Usage (0x21)
        0x95, 0x2C,        //   Report Count (44)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x06, 0x80, 0xFF,  //   Usage Page (Vendor Defined 0xFF80)
        0x85, 0x80,        //   Report ID (128)
        0x09, 0x20,        //   Usage (0x20)
        0x95, 0x06,        //   Report Count (6)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x81,        //   Report ID (129)
        0x09, 0x21,        //   Usage (0x21)
        0x95, 0x06,        //   Report Count (6)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x82,        //   Report ID (130)
        0x09, 0x22,        //   Usage (0x22)
        0x95, 0x05,        //   Report Count (5)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x83,        //   Report ID (131)
        0x09, 0x23,        //   Usage (0x23)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x84,        //   Report ID (132)
        0x09, 0x24,        //   Usage (0x24)
        0x95, 0x04,        //   Report Count (4)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x85,        //   Report ID (133)
        0x09, 0x25,        //   Usage (0x25)
        0x95, 0x06,        //   Report Count (6)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x86,        //   Report ID (134)
        0x09, 0x26,        //   Usage (0x26)
        0x95, 0x06,        //   Report Count (6)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x87,        //   Report ID (135)
        0x09, 0x27,        //   Usage (0x27)
        0x95, 0x23,        //   Report Count (35)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x88,        //   Report ID (136)
        0x09, 0x28,        //   Usage (0x28)
        0x95, 0x22,        //   Report Count (34)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x89,        //   Report ID (137)
        0x09, 0x29,        //   Usage (0x29)
        0x95, 0x02,        //   Report Count (2)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x90,        //   Report ID (144)
        0x09, 0x30,        //   Usage (0x30)
        0x95, 0x05,        //   Report Count (5)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x91,        //   Report ID (145)
        0x09, 0x31,        //   Usage (0x31)
        0x95, 0x03,        //   Report Count (3)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x92,        //   Report ID (146)
        0x09, 0x32,        //   Usage (0x32)
        0x95, 0x03,        //   Report Count (3)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0x93,        //   Report ID (147)
        0x09, 0x33,        //   Usage (0x33)
        0x95, 0x0C,        //   Report Count (12)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA0,        //   Report ID (160)
        0x09, 0x40,        //   Usage (0x40)
        0x95, 0x06,        //   Report Count (6)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA1,        //   Report ID (161)
        0x09, 0x41,        //   Usage (0x41)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA2,        //   Report ID (162)
        0x09, 0x42,        //   Usage (0x42)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA3,        //   Report ID (163)
        0x09, 0x43,        //   Usage (0x43)
        0x95, 0x30,        //   Report Count (48)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA4,        //   Report ID (164)
        0x09, 0x44,        //   Usage (0x44)
        0x95, 0x0D,        //   Report Count (13)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA5,        //   Report ID (165)
        0x09, 0x45,        //   Usage (0x45)
        0x95, 0x15,        //   Report Count (21)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA6,        //   Report ID (166)
        0x09, 0x46,        //   Usage (0x46)
        0x95, 0x15,        //   Report Count (21)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xF0,        //   Report ID (240)
        0x09, 0x47,        //   Usage (0x47)
        0x95, 0x3F,        //   Report Count (63)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xF1,        //   Report ID (241)
        0x09, 0x48,        //   Usage (0x48)
        0x95, 0x3F,        //   Report Count (63)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xF2,        //   Report ID (242)
        0x09, 0x49,        //   Usage (0x49)
        0x95, 0x0F,        //   Report Count (15)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA7,        //   Report ID (167)
        0x09, 0x4A,        //   Usage (0x4A)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA8,        //   Report ID (168)
        0x09, 0x4B,        //   Usage (0x4B)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xA9,        //   Report ID (169)
        0x09, 0x4C,        //   Usage (0x4C)
        0x95, 0x08,        //   Report Count (8)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAA,        //   Report ID (170)
        0x09, 0x4E,        //   Usage (0x4E)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAB,        //   Report ID (171)
        0x09, 0x4F,        //   Usage (0x4F)
        0x95, 0x39,        //   Report Count (57)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAC,        //   Report ID (172)
        0x09, 0x50,        //   Usage (0x50)
        0x95, 0x39,        //   Report Count (57)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAD,        //   Report ID (173)
        0x09, 0x51,        //   Usage (0x51)
        0x95, 0x0B,        //   Report Count (11)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAE,        //   Report ID (174)
        0x09, 0x52,        //   Usage (0x52)
        0x95, 0x01,        //   Report Count (1)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xAF,        //   Report ID (175)
        0x09, 0x53,        //   Usage (0x53)
        0x95, 0x02,        //   Report Count (2)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x85, 0xB0,        //   Report ID (176)
        0x09, 0x54,        //   Usage (0x54)
        0x95, 0x3F,        //   Report Count (63)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              // End Collection
    };


    CONST HID_DESCRIPTOR G_DefaultHidDescriptor = {
        0x09,   // length of HID descriptor
        0x21,   // descriptor type == HID  0x21
        0x0100, // hid spec release
        0x00,   // country code == Not Specified
        0x01,   // number of HID class descriptors
    { 0x22,   // descriptor type 
    sizeof(G_DefaultReportDescriptor) }  // total length of report descriptor
    };

    switch (IoControlCode)
    {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_DEVICE_DESCRIPTOR");

        status = WdfRequestRetrieveOutputMemory(Request, &memory);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfRequestRetrieveOutputMemory failed with status %!STATUS!",
                status);
            break;
        }

        bytesToCopy = G_DefaultHidDescriptor.bLength;

        status = WdfMemoryCopyFromBuffer(memory,
            0, // Offset
            (PVOID)&G_DefaultHidDescriptor,
            bytesToCopy);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfMemoryCopyFromBuffer failed with status %!STATUS!",
                status);
            break;
        }

        //
        // Report how many bytes were copied
        //
        WdfRequestSetInformation(Request, bytesToCopy);

        break;
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_REPORT_DESCRIPTOR");

        status = WdfRequestRetrieveOutputMemory(Request, &memory);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfRequestRetrieveOutputMemory failed with status %!STATUS!",
                status);
            break;
        }

        //
        // Use hardcoded Report descriptor
        //
        bytesToCopy = G_DefaultHidDescriptor.DescriptorList[0].wReportLength;

        if (bytesToCopy == 0) {
            status = STATUS_INVALID_DEVICE_STATE;
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "G_DefaultHidDescriptor's reportLenght is zero, %!STATUS!",
                status);
            break;
        }

        status = WdfMemoryCopyFromBuffer(memory,
            0,
            (PVOID)G_DefaultReportDescriptor,
            bytesToCopy);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfMemoryCopyFromBuffer failed  with status %!STATUS!",
                status);
            break;
        }

        //
        // Report how many bytes were copied
        //
        WdfRequestSetInformation(Request, bytesToCopy);

        break;
    case IOCTL_HID_READ_REPORT:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_READ_REPORT");

        status = STATUS_UNSUCCESSFUL;

        break;
    default:
        TraceEvents(TRACE_LEVEL_WARNING,
            TRACE_WIREBUS,
            "Unhandled IoControlCode: 0x%X",
            IoControlCode);
        break;
    }

    WdfRequestComplete(Request, status);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

NTSTATUS WireBusSetChildHandle(WDFDEVICE Device, PBD_ADDR Address, PBTH_HANDLE Handle)
{
    NTSTATUS                                status;
    WDFCHILDLIST                            list;
    PDO_IDENTIFICATION_DESCRIPTION          childIdDesc;
    PDO_ADDRESS_DESCRIPTION                 childAddrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    list = WdfFdoGetDefaultChildList(Device);

    WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
        &childIdDesc.Header,
        sizeof(PDO_IDENTIFICATION_DESCRIPTION)
    );
    WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(
        &childAddrDesc.Header,
        sizeof(PDO_ADDRESS_DESCRIPTION)
    );

    childIdDesc.ClientAddress = *Address;
    childAddrDesc.ClientHandle = *Handle;

    status = WdfChildListAddOrUpdateChildDescriptionAsPresent(
        list,
        &childIdDesc.Header,
        &childAddrDesc.Header
    );

    status = (status == STATUS_OBJECT_NAME_EXISTS) ? STATUS_SUCCESS : status;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit with status %!STATUS!", status);

    return status;
}
