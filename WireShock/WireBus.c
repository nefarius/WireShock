#include "Driver.h"
#include "wirebus.tmh"


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
    DECLARE_CONST_UNICODE_STRING(deviceId, L"USB\\VID_045E&PID_028E&REV_0114");
    DECLARE_CONST_UNICODE_STRING(instanceId, L"0ECE138");

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

    status = WdfPdoInitAddHardwareID(ChildInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAddHardwareID failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoInitAssignInstanceID(ChildInit, &instanceId);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoInitAssignInstanceID failed with status %!STATUS!",
            status);
        return status;
    }

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

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit with status %!STATUS!", status);

    return status;
}