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

#include "Driver.h"
#include "wirebus.tmh"
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <DsHid.h>


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
    WDF_TIMER_CONFIG                outTimerCfg;
    WDF_OBJECT_ATTRIBUTES           outTimerAttribs;

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
        pDesc->ClientAddress.Address[5],
        pDesc->ClientAddress.Address[4],
        pDesc->ClientAddress.Address[3],
        pDesc->ClientAddress.Address[2],
        pDesc->ClientAddress.Address[1],
        pDesc->ClientAddress.Address[0]);

#pragma region PDO Properties

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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoInitAssignDeviceID failed with status %!STATUS!",
            status);
        return status;
    }

    //
    // NOTE: same string is used to initialize hardware id too
    //
    status = WdfPdoInitAddHardwareID(ChildInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoInitAddHardwareID failed with status %!STATUS!",
            status);
        return status;
    }

    status = RtlUnicodeStringPrintf(&buffer,
        L"%02X:%02X:%02X:%02X:%02X:%02X",
        pDesc->ClientAddress.Address[5],
        pDesc->ClientAddress.Address[4],
        pDesc->ClientAddress.Address[3],
        pDesc->ClientAddress.Address[2],
        pDesc->ClientAddress.Address[1],
        pDesc->ClientAddress.Address[0]);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "RtlUnicodeStringPrintf failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfPdoInitAssignInstanceID(ChildInit, &buffer);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoInitAddDeviceText failed with status %!STATUS!",
            status);
        return status;
    }

    WdfPdoInitSetDefaultLocale(ChildInit, 0x409);

#pragma endregion

#pragma region Child device creation

    WDF_OBJECT_ATTRIBUTES_INIT(&pdoAttributes);

    status = WdfDeviceCreate(&ChildInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
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

    WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

    status = WdfPdoRetrieveAddressDescription(hChild, &addrDesc.Header);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
            status);
        return status;
    }

#pragma region HID Input Report Queue creation

    WDF_IO_QUEUE_CONFIG_INIT(&inputQueueCfg, WdfIoQueueDispatchManual);
    status = WdfIoQueueCreate(
        hChild,
        &inputQueueCfg,
        WDF_NO_OBJECT_ATTRIBUTES,
        &addrDesc.ChildDevice.HidInputReportQueue
    );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfIoQueueCreate failed with status %!STATUS!",
            status);
        return status;
    }

#pragma endregion

#pragma region HID Output Report Timer creation

    WDF_TIMER_CONFIG_INIT_PERIODIC(
        &outTimerCfg,
        WireChildOutputReportEvtTimerFunc,
        DS_ORT_START_DELAY
    );
    WDF_OBJECT_ATTRIBUTES_INIT(&outTimerAttribs);
    outTimerAttribs.ParentObject = hChild;

    status = WdfTimerCreate(
        &outTimerCfg,
        &outTimerAttribs,
        &addrDesc.ChildDevice.OutputReportTimer
    );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DRIVER,
            "WdfTimerCreate failed with status %!STATUS!",
            status);
        return status;
    }

#pragma endregion

    status = WdfPdoUpdateAddressDescription(hChild, &addrDesc.Header);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoUpdateAddressDescription failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exited with status %!STATUS!", status);

    return status;
}

_Use_decl_annotations_
VOID
WireShockEvtWdfChildListAddressDescriptionCleanup(
    WDFCHILDLIST  ChildList,
    PWDF_CHILD_ADDRESS_DESCRIPTION_HEADER  AddressDescription
)
{
    PPDO_ADDRESS_DESCRIPTION    pAddrDesc
        = CONTAINING_RECORD(
            AddressDescription,
            PDO_ADDRESS_DESCRIPTION,
            Header
        );

    UNREFERENCED_PARAMETER(ChildList);

    if (pAddrDesc->ChildDevice.RemoteName != NULL) {
        ExFreePoolWithTag(pAddrDesc->ChildDevice.RemoteName, WIRESHOCK_POOL_TAG);
        pAddrDesc->ChildDevice.RemoteName = NULL;
    }

    if (pAddrDesc->ChildDevice.OutputReportBuffer != NULL) {
        ExFreePoolWithTag(pAddrDesc->ChildDevice.OutputReportBuffer, WIRESHOCK_POOL_TAG);
        pAddrDesc->ChildDevice.OutputReportBuffer = NULL;
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

    NTSTATUS                            status = STATUS_SUCCESS;
    size_t                              bytesToCopy = 0;
    WDFMEMORY                           memory;
    WDFDEVICE                           device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));
    PDO_ADDRESS_DESCRIPTION             addrDesc;
    HID_XFER_PACKET                     packet;
    PDS_FEATURE_GET_HOST_BD_ADDR        pGetHostBdAddr;
    PDS_FEATURE_GET_DEVICE_BD_ADDR      pGetDeviceBdAddr;
    PDS_FEATURE_GET_DEVICE_TYPE         pGetDeviceType;
    PDS_FEATURE_GET_CONNECTION_TYPE     pGetConnectionType;
    PDEVICE_CONTEXT                     pParentCtx = DeviceGetContext(WdfPdoGetParent(device));
    PDO_IDENTIFICATION_DESCRIPTION      identDesc;


    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry (IoControlCode: 0x%X)", IoControlCode);

    switch (IoControlCode)
    {
#pragma region IOCTL_HID_GET_DEVICE_DESCRIPTOR

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

        bytesToCopy = G_Ds3HidDescriptor.bLength;

        status = WdfMemoryCopyFromBuffer(memory,
            0, // Offset
            (PVOID)&G_Ds3HidDescriptor,
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

#pragma endregion

#pragma region IOCTL_HID_GET_REPORT_DESCRIPTOR

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
        bytesToCopy = G_Ds3HidDescriptor.DescriptorList[0].wReportLength;

        if (bytesToCopy == 0) {
            status = STATUS_INVALID_DEVICE_STATE;
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "G_DefaultHidDescriptor's reportLength is zero, %!STATUS!",
                status);
            break;
        }

        status = WdfMemoryCopyFromBuffer(memory,
            0,
            (PVOID)G_Ds3HidReportDescriptor,
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

#pragma endregion

#pragma region IOCTL_HID_GET_DEVICE_ATTRIBUTES

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_DEVICE_ATTRIBUTES");

        //
        // TODO: implement, you lazy bum!
        // 
        status = STATUS_SUCCESS;

        break;

#pragma endregion

#pragma region IOCTL_HID_READ_REPORT

    case IOCTL_HID_READ_REPORT:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_READ_REPORT");

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

#pragma endregion

#pragma region IOCTL_HID_GET_FEATURE

    case IOCTL_HID_GET_FEATURE:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_FEATURE");

        status = DsHidRequestGetHidXferPacket_ToReadFromDevice(Request, &packet);

        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "Failed to read feature request (%!STATUS!)",
                status);
            break;
        }

        switch (packet.reportId)
        {
        case DS_FEATURE_TYPE_GET_HOST_BD_ADDR:

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_WIREBUS,
                ">> >> DS_FEATURE_TYPE_GET_HOST_BD_ADDR");

            pGetHostBdAddr = (PDS_FEATURE_GET_HOST_BD_ADDR)packet.reportBuffer;
            pGetHostBdAddr->HostAddress = pParentCtx->BluetoothHostAddress;
            REVERSE_BYTE_ARRAY(pGetHostBdAddr->HostAddress.Address, sizeof(BD_ADDR));

            break;
        case DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR:

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_WIREBUS,
                ">> >> DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR");

            WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(&identDesc.Header, sizeof(identDesc));

            status = WdfPdoRetrieveIdentificationDescription(device, &identDesc.Header);
            if (!NT_SUCCESS(status)) {
                TraceEvents(TRACE_LEVEL_ERROR,
                    TRACE_WIREBUS,
                    "WdfPdoRetrieveIdentificationDescription failed with status %!STATUS!",
                    status);
                break;
            }

            pGetDeviceBdAddr = (PDS_FEATURE_GET_DEVICE_BD_ADDR)packet.reportBuffer;
            pGetDeviceBdAddr->DeviceAddress = identDesc.ClientAddress;
            REVERSE_BYTE_ARRAY(pGetDeviceBdAddr->DeviceAddress.Address, sizeof(BD_ADDR));

            break;
        case DS_FEATURE_TYPE_GET_DEVICE_TYPE:

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_WIREBUS,
                ">> >> DS_FEATURE_TYPE_GET_DEVICE_TYPE");

            WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

            status = WdfPdoRetrieveAddressDescription(device, &addrDesc.Header);
            if (!NT_SUCCESS(status)) {
                TraceEvents(TRACE_LEVEL_ERROR,
                    TRACE_WIREBUS,
                    "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
                    status);
                break;
            }

            pGetDeviceType = (PDS_FEATURE_GET_DEVICE_TYPE)packet.reportBuffer;
            pGetDeviceType->DeviceType = addrDesc.ChildDevice.DeviceType;

            break;
        case DS_FEATURE_TYPE_GET_CONNECTION_TYPE:

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_WIREBUS,
                ">> >> DS_FEATURE_TYPE_GET_DEVICE_TYPE");

            pGetConnectionType = (PDS_FEATURE_GET_CONNECTION_TYPE)packet.reportBuffer;
            pGetConnectionType->ConnectionType = DS_CONNECTION_TYPE_BLUETOOTH;

            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR, //TODO: CHECK ME
                TRACE_WIREBUS,
                "%!FUNC!: Unknown IOCTL_HID_GET_FEATURE reportId %d, Default case triggered",
                packet.reportId);
            break;
        }

        WdfRequestSetInformation(Request, packet.reportBufferLen);

        break;

#pragma endregion

#pragma region IOCTL_HID_SET_FEATURE

    case IOCTL_HID_SET_FEATURE:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE,
            ">> IOCTL_HID_SET_FEATURE");

        status = DsHidRequestGetHidXferPacket_ToWriteToDevice(Request, &packet);

        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE,
                "Failed to read feature request (%!STATUS!)",
                status);
            break;
        }

        switch (packet.reportId)
        {
        case DS_FEATURE_TYPE_SET_HOST_BD_ADDR:

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_QUEUE,
                ">> >> DS_FEATURE_TYPE_SET_HOST_BD_ADDR");

            status = STATUS_NOT_SUPPORTED;

            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR, //TODO: CHECK ME
                TRACE_WIREBUS, //TODO: CHECK ME
                "%!FUNC!: Unknown IOCTL_HID_SET_FEATURE reportId %d, Default case triggered",
                packet.reportId);
            break;
        }

        break;

#pragma endregion

#pragma region IOCTL_HID_GET_STRING

    case IOCTL_HID_GET_STRING:                      // METHOD_NEITHER

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_STRING");

        WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

        status = WdfPdoRetrieveAddressDescription(device, &addrDesc.Header);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
                status);
            break;
        }

        status = DsHidGetString(Request, addrDesc.ChildDevice.DeviceType);
        break;

#pragma endregion

#pragma region IOCTL_HID_GET_INDEXED_STRING

    case IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_WIREBUS,
            ">> IOCTL_HID_GET_INDEXED_STRING");

        WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

        status = WdfPdoRetrieveAddressDescription(device, &addrDesc.Header);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS,
                "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
                status);
            break;
        }

        status = DsHidGetIndexedString(Request, addrDesc.ChildDevice.DeviceType);
        break;

#pragma endregion

    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:  // METHOD_NEITHER
                                                    //
                                                    // This has the USBSS Idle notification callback. If the lower driver
                                                    // can handle it (e.g. USB stack can handle it) then pass it down,
                                                    // otherwise complete it here as not implemented. For a virtual
                                                    // device, idling is not needed.
                                                    //
                                                    // Not implemented. Fall through...
                                                    //
    case IOCTL_HID_ACTIVATE_DEVICE:                 // METHOD_NEITHER
    case IOCTL_HID_DEACTIVATE_DEVICE:               // METHOD_NEITHER
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:             // METHOD_OUT_DIRECT
                                                    //
                                                    // We don't do anything for these IOCTLs but some minidrivers might.
                                                    //
                                                    // Not implemented. Fall through...
                                                    //
    default:
        TraceEvents(TRACE_LEVEL_WARNING,
            TRACE_WIREBUS,
            "Unhandled IoControlCode: 0x%X",
            IoControlCode);
        status = STATUS_NOT_IMPLEMENTED;
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

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

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

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");

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
    WDFCHILDLIST                    list = WdfFdoGetDefaultChildList(Device);
    WDF_CHILD_LIST_ITERATOR         iter;
    WDF_CHILD_RETRIEVE_INFO         info;
    PDO_IDENTIFICATION_DESCRIPTION  desc;
    WDFDEVICE                       child;
    BOOLEAN                         retval = FALSE;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

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

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");

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

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

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

    status = ((status == STATUS_OBJECT_NAME_EXISTS) ? STATUS_SUCCESS : status);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfChildListAddOrUpdateChildDescriptionAsPresent failed with status %!STATUS!",
            status);
        return FALSE;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");

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

VOID WireBusSetChildDeviceType(WDFDEVICE Device, PBD_ADDR Address, DS_DEVICE_TYPE DeviceType)
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

VOID WireBusInitChildOutputReport(WDFDEVICE Device, PBD_ADDR Address)
{
    PDO_ADDRESS_DESCRIPTION                 childAddrDesc;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    if (WireBusGetPdoAddressDescription(Device, Address, &childAddrDesc))
    {
        if (childAddrDesc.ChildDevice.OutputReportBuffer != NULL) {
            ExFreePoolWithTag(childAddrDesc.ChildDevice.OutputReportBuffer, WIRESHOCK_POOL_TAG);
        }

        switch (childAddrDesc.ChildDevice.DeviceType)
        {
        case DS_DEVICE_TYPE_PS3_DUALSHOCK:

            childAddrDesc.ChildDevice.OutputReportBuffer = ExAllocatePoolWithTag(
                NonPagedPoolNx,
                DS3_HID_OUTPUT_REPORT_SIZE,
                WIRESHOCK_POOL_TAG
            );
            RtlCopyMemory(
                childAddrDesc.ChildDevice.OutputReportBuffer,
                G_Ds3HidOutputReport,
                DS3_HID_OUTPUT_REPORT_SIZE
            );

            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_WIREBUS, //TODO: CHECK ME
                "%!FUNC!: Unknown DeviceType %d, Default case triggered",
                childAddrDesc.ChildDevice.DeviceType);
            break;
        }

        WireBusSetPdoAddressDescription(Device, Address, &childAddrDesc);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}

_Use_decl_annotations_
VOID
WireChildOutputReportEvtTimerFunc(
    WDFTIMER  Timer
)
{
    NTSTATUS                    status;
    WDFDEVICE                   device = WdfTimerGetParentObject(Timer);
    PDO_ADDRESS_DESCRIPTION     addrDesc;
    L2CAP_CID                   scid;
    PDEVICE_CONTEXT             pParentCtx = DeviceGetContext(WdfPdoGetParent(device));

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Entry");

    WDF_CHILD_ADDRESS_DESCRIPTION_HEADER_INIT(&addrDesc.Header, sizeof(addrDesc));

    status = WdfPdoRetrieveAddressDescription(device, &addrDesc.Header);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS,
            "WdfPdoRetrieveAddressDescription failed with status %!STATUS!",
            status);
        return;
    }

    L2CAP_DEVICE_GET_SCID_FOR_TYPE(
        &addrDesc.ChildDevice,
        L2CAP_PSM_HID_Command,
        &scid);

    switch (addrDesc.ChildDevice.DeviceType)
    {
    case DS_DEVICE_TYPE_PS3_DUALSHOCK:

        status = HID_Command(
            pParentCtx,
            addrDesc.ChildDevice.HCI_ConnectionHandle,
            scid,
            addrDesc.ChildDevice.OutputReportBuffer,
            DS3_HID_OUTPUT_REPORT_SIZE);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE,
                "HID_Command failed with status %!STATUS!",
                status);
        }

        break;
    default:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_WIREBUS, //TODO: CHECK ME
            "%!FUNC!: Unknown DeviceType %d, Default case triggered",
            addrDesc.ChildDevice.DeviceType);
        break;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_WIREBUS, "%!FUNC! Exit");
}
