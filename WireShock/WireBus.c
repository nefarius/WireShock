#include "Driver.h"
#include "wirebus.tmh"
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <usbioctl.h>


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

    DECLARE_CONST_UNICODE_STRING(compatId00, L"USB\\Class_03&SubClass_00&Prot_00");
    DECLARE_CONST_UNICODE_STRING(compatId01, L"USB\\Class_03&SubClass_00");
    DECLARE_CONST_UNICODE_STRING(compatId02, L"USB\\Class_03");
    DECLARE_CONST_UNICODE_STRING(deviceLocation, L"Toaster Bus 0");
    DECLARE_UNICODE_STRING_SIZE(buffer, 1024);
    DECLARE_UNICODE_STRING_SIZE(deviceId, 1024);
    PCWSTR HardwareIds = L"USB\\VID_045E&PID_028E&REV_0114\0USB\\VID_045E&PID_028E\0\0";
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

    status = WdfPdoInitAddCompatibleID(ChildInit, &compatId00);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddCompatibleID (#0) failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoInitAddCompatibleID(ChildInit, &compatId01);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddCompatibleID (#1) failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoInitAddCompatibleID(ChildInit, &compatId02);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddCompatibleID (#2) failed with status %!STATUS!",
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
        L"Microsoft_Eliyas_Toaster_%02d",
        SerialNo);
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
    UNREFERENCED_PARAMETER(IoControlCode);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    TraceEvents(TRACE_LEVEL_INFORMATION, 
        TRACE_WIREBUS,
        "IoControlCode: 0x%X, 0x%X",
        IoControlCode, IOCTL_INTERNAL_USB_SUBMIT_URB);

    WdfRequestComplete(Request, STATUS_SUCCESS);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}
