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
#include "bulkrwr.tmh"

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
WireShockConfigContReaderForBulkReadEndPoint(
    _In_ WDFDEVICE Device
)
{
    WDF_USB_CONTINUOUS_READER_CONFIG    contReaderConfig;
    NTSTATUS                            status;
    PDEVICE_CONTEXT                     pDeviceCtx;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Entry");

    pDeviceCtx = DeviceGetContext(Device);

    WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&contReaderConfig,
        WireShockEvtUsbBulkReadPipeReadComplete,
        Device,    // Context
        BULK_IN_BUFFER_LENGTH);   // TransferLength

    contReaderConfig.EvtUsbTargetPipeReadersFailed = WireShockEvtUsbBulkReadReadersFailed;

    //
    // Reader requests are not posted to the target automatically.
    // Driver must explictly call WdfIoTargetStart to kick start the
    // reader.  In this sample, it's done in D0Entry.
    // By defaut, framework queues two requests to the target
    // endpoint. Driver can configure up to 10 requests with CONFIG macro.
    //
    status = WdfUsbTargetPipeConfigContinuousReader(pDeviceCtx->BulkReadPipe,
        &contReaderConfig);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "WdfUsbTargetPipeConfigContinuousReader failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Exit");

    return status;
}

NTSTATUS WriteBulkPipe(
    PDEVICE_CONTEXT Context,
    PVOID Buffer,
    ULONG BufferLength)
{
    NTSTATUS                        status;
    WDFREQUEST                      request;
    WDF_OBJECT_ATTRIBUTES           attribs;
    WDFMEMORY                       memory;
    PVOID                           writeBufferPointer;

    WDF_OBJECT_ATTRIBUTES_INIT(&attribs);

    status = WdfRequestCreate(&attribs,
        WdfUsbTargetDeviceGetIoTarget(Context->UsbDevice),
        &request);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "WdfRequestCreate failed with status %!STATUS!",
            status);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
    attribs.ParentObject = request;

    status = WdfMemoryCreate(&attribs,
        NonPagedPoolNx,
        WIRESHOCK_POOL_TAG,
        BufferLength,
        &memory,
        &writeBufferPointer);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "WdfMemoryCreate failed with status %!STATUS!",
            status);
        return status;
    }

    RtlCopyMemory(writeBufferPointer, Buffer, BufferLength);

    status = WdfUsbTargetPipeFormatRequestForWrite(
        Context->BulkWritePipe,
        request,
        memory,
        NULL
    );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "WdfUsbTargetPipeFormatRequestForWrite failed with status %!STATUS!",
            status);
        return status;
    }

    WdfRequestSetCompletionRoutine(
        request, 
        EvtUsbRequestCompletionRoutine,
        NULL
    );

    if (WdfRequestSend(request,
        WdfUsbTargetDeviceGetIoTarget(Context->UsbDevice),
        NULL) == FALSE)
    {
        status = WdfRequestGetStatus(request);
    }

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "WdfRequestSend failed with status %!STATUS!",
            status);
    }

    return status;
}

NTSTATUS
HID_Command(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    L2CAP_CID Channel,
    PVOID Buffer,
    ULONG BufferLength
)
{
    NTSTATUS status;
    PUCHAR buffer = ExAllocatePoolWithTag(NonPagedPoolNx, BufferLength + 8, WIRESHOCK_POOL_TAG);

    buffer[0] = Handle.Lsb;
    buffer[1] = Handle.Msb;
    buffer[2] = (BYTE)((BufferLength + 4) % 256);
    buffer[3] = (BYTE)((BufferLength + 4) / 256);
    buffer[4] = (BYTE)(BufferLength % 256);
    buffer[5] = (BYTE)(BufferLength / 256);
    buffer[6] = Channel.Lsb;
    buffer[7] = Channel.Msb;

    RtlCopyMemory(&buffer[8], Buffer, BufferLength);

    status = WriteBulkPipe(Context, buffer, BufferLength + 8);

    ExFreePoolWithTag(buffer, WIRESHOCK_POOL_TAG);

    return status;
}

VOID
WireShockEvtUsbBulkReadPipeReadComplete(
    WDFUSBPIPE  Pipe,
    WDFMEMORY   Buffer,
    size_t      NumBytesTransferred,
    WDFCONTEXT  Context
)
{
    NTSTATUS                        status;
    WDFDEVICE                       device;
    PDEVICE_CONTEXT                 pDeviceContext;
    PUCHAR                          buffer;
    BTH_HANDLE                      clientHandle;
    PBTH_DEVICE                     pClientDevice;
    L2CAP_SIGNALLING_COMMAND_CODE   code;
    PDO_ADDRESS_DESCRIPTION         addrDesc;
    BD_ADDR                         clientAddr;

    static BYTE CID = 0x01;

    UNREFERENCED_PARAMETER(Pipe);

    if (NumBytesTransferred == 0) {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_BULKRWR,
            "!FUNC! Zero length read "
            "occurred on the Interrupt Pipe's Continuous Reader\n"
        );
        return;
    }

    device = Context;
    pDeviceContext = DeviceGetContext(device);
    buffer = WdfMemoryGetBuffer(Buffer, NULL);

    BTH_HANDLE_FROM_BUFFER(clientHandle, buffer);

    //
    // Fetch child device matching connection handle
    // 
    if (!WireBusGetPdoAddressDescriptionByHandle(device, &clientHandle, &addrDesc, &clientAddr)) {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_BULKRWR,
            "WireBusGetPdoAddressDescriptionByHandle failed for %02X %02X",
            clientHandle.Lsb, clientHandle.Msb
        );
        return;
    }

    pClientDevice = &addrDesc.ChildDevice;

    if (pClientDevice == NULL)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "PBTH_DEVICE not found");
        return;
    }

    if (L2CAP_IS_CONTROL_CHANNEL(buffer))
    {
        if (L2CAP_IS_SIGNALLING_COMMAND_CODE(buffer))
        {
            code = L2CAP_GET_SIGNALLING_COMMAND_CODE(buffer);

            switch (code)
            {
            case L2CAP_Command_Reject:
            {
                PL2CAP_SIGNALLING_COMMAND_REJECT data = (PL2CAP_SIGNALLING_COMMAND_REJECT)&buffer[8];

                TraceEvents(TRACE_LEVEL_WARNING, 
                    TRACE_BULKRWR, 
                    ">> L2CAP_Command_Reject: 0x%04X", 
                    data->Reason);

                break;
            }

#pragma region L2CAP_Connection_Request

            case L2CAP_Connection_Request:

                status = Ds3ConnectionRequest(
                    pDeviceContext,
                    pClientDevice,
                    buffer,
                    &CID);

                break;

#pragma endregion

#pragma region L2CAP_Connection_Response

            case L2CAP_Connection_Response:

                status = Ds3ConnectionResponse(
                    buffer
                );

                break;

#pragma endregion

#pragma region L2CAP_Configuration_Request

            case L2CAP_Configuration_Request:

                status = Ds3ConfigurationRequest(
                    pDeviceContext,
                    pClientDevice,
                    buffer);

                break;

#pragma endregion

#pragma region L2CAP_Configuration_Response

            case L2CAP_Configuration_Response:

                status = Ds3ConfigurationResponse(
                    pDeviceContext,
                    pClientDevice,
                    buffer
                );

                break;

#pragma endregion

#pragma region L2CAP_Disconnection_Request

            case L2CAP_Disconnection_Request:

                status = Ds3DisconnectionRequest(
                    pDeviceContext,
                    pClientDevice,
                    buffer);

                break;

#pragma endregion

#pragma region L2CAP_Disconnection_Response

            case L2CAP_Disconnection_Response:

                TraceEvents(TRACE_LEVEL_WARNING, 
                    TRACE_BULKRWR, 
                    ">> L2CAP_Disconnection_Response");

                break;

#pragma endregion

            default:
                TraceEvents(TRACE_LEVEL_WARNING, 
                    TRACE_BULKRWR, 
                    "Unknown L2CAP command: 0x%02X", 
                    code);
                break;
            }
        }
    }
    else if (L2CAP_IS_HID_INPUT_REPORT(buffer))
    {
        switch (pClientDevice->DeviceType)
        {
        case DS_DEVICE_TYPE_PS3_DUALSHOCK:

            status = Ds3ProcessHidInputReport(pClientDevice, buffer);

            break;
        default:

            TraceEvents(TRACE_LEVEL_WARNING, 
                TRACE_BULKRWR, 
                "Unknown DS_DEVICE_TYPE: 0x%02X", 
                pClientDevice->DeviceType);

            break;
        }
    }

    //
    // Write back state changes
    // 
    if (!WireBusSetPdoAddressDescription(device, &clientAddr, &addrDesc)) {
        TraceEvents(TRACE_LEVEL_WARNING,
            TRACE_BULKRWR,
            "WireBusSetPdoAddressDescription failed for %02X:%02X:%02X:%02X:%02X:%02X",
            clientAddr.Address[0],
            clientAddr.Address[1],
            clientAddr.Address[2],
            clientAddr.Address[3],
            clientAddr.Address[4],
            clientAddr.Address[5]
        );
    }
}

BOOLEAN
WireShockEvtUsbBulkReadReadersFailed(
    _In_ WDFUSBPIPE Pipe,
    _In_ NTSTATUS Status,
    _In_ USBD_STATUS UsbdStatus
)
{
    UNREFERENCED_PARAMETER(Pipe);

    TraceEvents(TRACE_LEVEL_ERROR,
        TRACE_BULKRWR,
        "Reading bulk endpoint failed with status %!STATUS! (UsbdStatus: 0x%X)",
        Status, UsbdStatus);

    return TRUE;
}
