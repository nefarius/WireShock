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
    WDF_IO_QUEUE_CONFIG             inputQueueCfg;
    PDO_ADDRESS_DESCRIPTION         addrDesc;

    DECLARE_CONST_UNICODE_STRING(deviceLocation, L"WireShock Bus Device");
    DECLARE_UNICODE_STRING_SIZE(buffer, MAX_DEVICE_ID_LEN);
    DECLARE_UNICODE_STRING_SIZE(deviceId, MAX_DEVICE_ID_LEN);
    PCWSTR HardwareIds = L"Nefarius\\WireShockHidDevice\0\0";


    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    pDesc = CONTAINING_RECORD(
        IdentificationDescription,
        PDO_IDENTIFICATION_DESCRIPTION,
        Header);

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_WIREBUS,
        "Plugging in device with address %02X:%02X:%02X:%02X:%02X:%02X",
        pDesc->ClientAddress.Address[0],
        pDesc->ClientAddress.Address[1],
        pDesc->ClientAddress.Address[2],
        pDesc->ClientAddress.Address[3],
        pDesc->ClientAddress.Address[4],
        pDesc->ClientAddress.Address[5]);

    //
    // PDO features
    // 
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

    status = RtlUnicodeStringPrintf(&buffer,
        L"%02X:%02X:%02X:%02X:%02X:%02X",
        pDesc->ClientAddress.Address[0],
        pDesc->ClientAddress.Address[1],
        pDesc->ClientAddress.Address[2],
        pDesc->ClientAddress.Address[3],
        pDesc->ClientAddress.Address[4],
        pDesc->ClientAddress.Address[5]);
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

#pragma region Child device creation

    WDF_OBJECT_ATTRIBUTES_INIT(&pdoAttributes);

    status = WdfDeviceCreate(&ChildInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfDeviceCreate failed with status %!STATUS!",
            status);
        return status;
    }

#pragma endregion

#pragma region PNP & Power Capabilities

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

#pragma endregion

#pragma region Default I/O Queue creation

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

#pragma endregion

#pragma region HID Input Report Queue creation

    WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

    status = WdfPdoRetrieveAddressDescription(hChild, &addrDesc.Header);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
            status);
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(&inputQueueCfg, WdfIoQueueDispatchManual);
    status = WdfIoQueueCreate(
        hChild, 
        &inputQueueCfg, 
        WDF_NO_OBJECT_ATTRIBUTES, 
        &addrDesc.ChildDevice.HidInputReportQueue
    );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfIoQueueCreate failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoUpdateAddressDescription(hChild, &addrDesc.Header);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_WIREBUS,
            "WdfPdoUpdateAddressDescription failed with status %!STATUS!",
            status);
        return status;
    }

#pragma endregion

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit with status %!STATUS!", status);

    return status;
}

_Use_decl_annotations_
VOID
WireShockEvtWdfChildListAddressDescriptionCleanup(
    WDFCHILDLIST  ChildList,
    PWDF_CHILD_ADDRESS_DESCRIPTION_HEADER  AddressDescription
)
{
    PPDO_ADDRESS_DESCRIPTION    pAddrDesc;

    UNREFERENCED_PARAMETER(ChildList);

    pAddrDesc = CONTAINING_RECORD(
        AddressDescription,
        PDO_ADDRESS_DESCRIPTION,
        Header
    );

    if (pAddrDesc->ChildDevice.RemoteName != NULL) {
        ExFreePoolWithTag(pAddrDesc->ChildDevice.RemoteName, WIRESHOCK_POOL_TAG);
        pAddrDesc->ChildDevice.RemoteName = NULL;
    }
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

    NTSTATUS                        status = STATUS_SUCCESS;
    size_t                          bytesToCopy = 0;
    WDFMEMORY                       memory;
    WDFDEVICE                       device;
    PDO_ADDRESS_DESCRIPTION         addrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry (IoControlCode: 0x%X)", IoControlCode);
        

    CONST HID_REPORT_DESCRIPTOR Ds3HidReportDescriptor[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x02,        // Collection (Logical)
        0xA1, 0x01,        //   Collection (Application)
        0x85, 0x01,        //     Report ID (1)
        0x09, 0x30,        //     Usage (X)
        0x09, 0x31,        //     Usage (Y)
        0x09, 0x32,        //     Usage (Z)
        0x09, 0x35,        //     Usage (Rz)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x04,        //     Report Count (4)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x09, 0x39,        //     Usage (Hat switch)
        0x15, 0x00,        //     Logical Minimum (0)
        0x25, 0x07,        //     Logical Maximum (7)
        0x35, 0x00,        //     Physical Minimum (0)
        0x46, 0x3B, 0x01,  //     Physical Maximum (315)
        0x65, 0x14,        //     Unit (System: English Rotation, Length: Centimeter)
        0x75, 0x04,        //     Report Size (4)
        0x95, 0x01,        //     Report Count (1)
        0x81, 0x42,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
        0x65, 0x00,        //     Unit (None)
        0x05, 0x09,        //     Usage Page (Button)
        0x19, 0x01,        //     Usage Minimum (0x01)
        0x29, 0x0E,        //     Usage Maximum (0x0E)
        0x15, 0x00,        //     Logical Minimum (0)
        0x25, 0x01,        //     Logical Maximum (1)
        0x75, 0x01,        //     Report Size (1)
        0x95, 0x0E,        //     Report Count (14)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x20,        //     Usage (0x20)
        0x75, 0x06,        //     Report Size (6)
        0x95, 0x01,        //     Report Count (1)
        0x15, 0x00,        //     Logical Minimum (0)
        0x25, 0x7F,        //     Logical Maximum (127)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x09, 0x33,        //     Usage (Rx)
        0x09, 0x34,        //     Usage (Ry)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x02,        //     Report Count (2)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0xA1, 0x01,        //   Collection (Application)
        0x85, 0x01,        //     Report ID (1)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,        //     Usage (0x01)
        0x09, 0x02,        //     Usage (0x02)
        0x09, 0x03,        //     Usage (0x03)
        0x09, 0x04,        //     Usage (0x04)
        0x09, 0x05,        //     Usage (0x05)
        0x09, 0x06,        //     Usage (0x06)
        0x09, 0x07,        //     Usage (0x07)
        0x09, 0x08,        //     Usage (0x08)
        0x09, 0x09,        //     Usage (0x09)
        0x09, 0x0A,        //     Usage (0x0A)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x0A,        //     Report Count (10)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xA1, 0x01,        //     Collection (Application)
        0x85, 0x01,        //       Report ID (1)
        0x06, 0x01, 0xFF,  //       Usage Page (Vendor Defined 0xFF01)
        0x09, 0x01,        //       Usage (0x01)
        0x75, 0x08,        //       Report Size (8)
        0x95, 0x1D,        //       Report Count (29)
        0x15, 0x00,        //       Logical Minimum (0)
        0x26, 0xFF, 0x00,  //       Logical Maximum (255)
        0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //     End Collection
        0xC0,              //   End Collection
        0xC0,              // End Collection

                           // 160 bytes
    };
    
    CONST HID_DESCRIPTOR Ds3HidDescriptor = {
        0x09,   // length of HID descriptor
        0x21,   // descriptor type == HID  0x21
        0x0100, // hid spec release
        0x00,   // country code == Not Specified
        0x01,   // number of HID class descriptors
    { 0x22,   // descriptor type 
    sizeof(Ds3HidReportDescriptor) }  // total length of report descriptor
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

        bytesToCopy = Ds3HidDescriptor.bLength;

        status = WdfMemoryCopyFromBuffer(memory,
            0, // Offset
            (PVOID)&Ds3HidDescriptor,
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
        bytesToCopy = Ds3HidDescriptor.DescriptorList[0].wReportLength;

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
            (PVOID)Ds3HidReportDescriptor,
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

        device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));

        WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

        status = WdfPdoRetrieveAddressDescription(device, &addrDesc.Header);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, 
                TRACE_WIREBUS,
                "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
                status);
            break;
        }

        status = WdfRequestForwardToIoQueue(Request, addrDesc.ChildDevice.HidInputReportQueue);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, 
                TRACE_WIREBUS,
                "WdfRequestForwardToIoQueue failed with status %!STATUS!",
                status);
            break;
        }

        status = STATUS_PENDING;

        break;
    default:
        TraceEvents(TRACE_LEVEL_WARNING,
            TRACE_WIREBUS,
            "Unhandled IoControlCode: 0x%X",
            IoControlCode);
        break;
    }

    if (status != STATUS_PENDING) {
        WdfRequestComplete(Request, status);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

BOOLEAN WireBusGetPdoAddressDescription(
    WDFDEVICE Device,
    PBD_ADDR Address,
    PPDO_ADDRESS_DESCRIPTION AddressDescription
)
{
    NTSTATUS                        status;
    PDO_IDENTIFICATION_DESCRIPTION  childIdDesc;

    WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
        &childIdDesc.Header,
        sizeof(PDO_IDENTIFICATION_DESCRIPTION)
    );
    WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(
        &AddressDescription->Header,
        sizeof(PDO_ADDRESS_DESCRIPTION)
    );

    childIdDesc.ClientAddress = *Address;

    status = WdfChildListRetrieveAddressDescription(
        WdfFdoGetDefaultChildList(Device),
        &childIdDesc.Header,
        &AddressDescription->Header);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfChildListRetrieveAddressDescription failed with status %!STATUS!",
            status);
        return FALSE;
    }

    return TRUE;
}

BOOLEAN WireBusGetPdoAddressDescriptionByHandle(
    WDFDEVICE Device,
    PBTH_HANDLE Handle,
    PPDO_ADDRESS_DESCRIPTION AddressDescription,
    PBD_ADDR Address
)
{
    NTSTATUS                        status;
    WDFCHILDLIST                    list;
    WDF_CHILD_LIST_ITERATOR         iter;
    WDF_CHILD_RETRIEVE_INFO         info;
    PDO_IDENTIFICATION_DESCRIPTION  desc;
    WDFDEVICE                       child;
    BOOLEAN                         retval = FALSE;


    list = WdfFdoGetDefaultChildList(Device);

    WDF_CHILD_LIST_ITERATOR_INIT(&iter, WdfRetrievePresentChildren);

    WdfChildListBeginIteration(list, &iter);

    //
    // Enumerate through child list
    // 
    for (;;)
    {
        WDF_CHILD_RETRIEVE_INFO_INIT(&info, &desc.Header);
        WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
            &desc.Header,
            sizeof(desc)
        );

        //
        // Fetch next child device
        // 
        status = WdfChildListRetrieveNextDevice(
            list,
            &iter,
            &child,
            &info
        );
        if (!NT_SUCCESS(status)
            || status == STATUS_NO_MORE_ENTRIES
            || info.Status != WdfChildListRetrieveDeviceSuccess)
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfChildListRetrieveNextDevice returned with status %!STATUS!",
                status);
            break;
        }

        //
        // Fetch address description of child device
        // 
        if (!WireBusGetPdoAddressDescription(Device, &desc.ClientAddress, AddressDescription)) {
            break;
        }

        //
        // Compare handle value and break on successful match
        // 
        if (RtlCompareMemory(
            &AddressDescription->ChildDevice.HCI_ConnectionHandle,
            Handle,
            sizeof(BTH_HANDLE)
        ) == sizeof(BTH_HANDLE))
        {
            if (Address != NULL) {
                *Address = desc.ClientAddress;
            }
            retval = TRUE;
            break;
        }
    }

    WdfChildListEndIteration(list, &iter);

    return retval;
}

BOOLEAN WireBusSetPdoAddressDescription(
    WDFDEVICE Device,
    PBD_ADDR Address,
    PPDO_ADDRESS_DESCRIPTION AddressDescription
)
{
    NTSTATUS                        status;
    PDO_IDENTIFICATION_DESCRIPTION  childIdDesc;

    WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
        &childIdDesc.Header,
        sizeof(PDO_IDENTIFICATION_DESCRIPTION)
    );

    childIdDesc.ClientAddress = *Address;

    status = WdfChildListAddOrUpdateChildDescriptionAsPresent(
        WdfFdoGetDefaultChildList(Device),
        &childIdDesc.Header,
        &AddressDescription->Header
    );

    status = (status == STATUS_OBJECT_NAME_EXISTS) ? STATUS_SUCCESS : status;

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfChildListAddOrUpdateChildDescriptionAsPresent failed with status %!STATUS!",
            status);
        return FALSE;
    }

    return TRUE;
}

VOID WireBusSetChildHandle(WDFDEVICE Device, PBD_ADDR Address, PBTH_HANDLE Handle)
{
    PDO_ADDRESS_DESCRIPTION                 childAddrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    if (WireBusGetPdoAddressDescription(Device, Address, &childAddrDesc))
    {
        childAddrDesc.ChildDevice.HCI_ConnectionHandle = *Handle;

        WireBusSetPdoAddressDescription(Device, Address, &childAddrDesc);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

VOID WireBusSetChildDeviceType(WDFDEVICE Device, PBD_ADDR Address, BTH_DEVICE_TYPE DeviceType)
{
    PDO_ADDRESS_DESCRIPTION                 childAddrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    if (WireBusGetPdoAddressDescription(Device, Address, &childAddrDesc))
    {
        childAddrDesc.ChildDevice.DeviceType = DeviceType;

        WireBusSetPdoAddressDescription(Device, Address, &childAddrDesc);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

VOID WireBusSetChildRemoteName(WDFDEVICE Device, PBD_ADDR Address, PUCHAR Buffer, ULONG BufferLength)
{
    PDO_ADDRESS_DESCRIPTION                 childAddrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    if (WireBusGetPdoAddressDescription(Device, Address, &childAddrDesc))
    {
        if (childAddrDesc.ChildDevice.RemoteName != NULL) {
            ExFreePoolWithTag(childAddrDesc.ChildDevice.RemoteName, WIRESHOCK_POOL_TAG);
        }
        childAddrDesc.ChildDevice.RemoteName = ExAllocatePoolWithTag(
            NonPagedPoolNx,
            BufferLength,
            WIRESHOCK_POOL_TAG
        );
        RtlCopyMemory(childAddrDesc.ChildDevice.RemoteName, Buffer, BufferLength);

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            "Set device name to: %s",
            childAddrDesc.ChildDevice.RemoteName);

        WireBusSetPdoAddressDescription(Device, Address, &childAddrDesc);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

