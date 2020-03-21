#include "UsbInterrupt.h"

NTSTATUS
SendControlRequest(
    _In_ WDFUSBDEVICE UsbDevice,
    _In_ WDF_USB_BMREQUEST_DIRECTION Direction,
    _In_ WDF_USB_BMREQUEST_TYPE Type,
    _In_ WDF_USB_BMREQUEST_RECIPIENT Recipient,
    _In_ BYTE Request,
    _In_ USHORT Value,
    _In_ USHORT Index,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _In_opt_ PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionRoutine,
    _In_opt_ __drv_aliasesMem WDFCONTEXT        CompletionContext)
{
    NTSTATUS                        status;
    WDF_USB_CONTROL_SETUP_PACKET    controlSetupPacket;
    WDFREQUEST                      request;
    WDF_OBJECT_ATTRIBUTES           attribs;
    WDFMEMORY                       memory;
    PVOID                           writeBufferPointer;

    WDF_OBJECT_ATTRIBUTES_INIT(&attribs);

    status = WdfRequestCreate(&attribs,
        WdfUsbTargetDeviceGetIoTarget(UsbDevice),
        &request);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
    attribs.ParentObject = request;

    status = WdfMemoryCreate(&attribs,
        NonPagedPool,
        USB_INTERRUPT_POOL_TAG,
        BufferLength,
        &memory,
        &writeBufferPointer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (Buffer == NULL) {
        RtlZeroMemory(writeBufferPointer, BufferLength);
    } else {
        RtlCopyMemory(writeBufferPointer, Buffer, BufferLength);
    }

    switch (Type)
    {
    case BmRequestClass:
        WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(&controlSetupPacket,
            Direction,
            Recipient,
            Request,
            Value,
            Index);
        break;

        // TODO: maybe add support other types

    default:
        return STATUS_INVALID_PARAMETER;
    }

    status = WdfUsbTargetDeviceFormatRequestForControlTransfer(
        UsbDevice,
        request,
        &controlSetupPacket,
        memory,
        NULL
    );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (CompletionRoutine) {
        WdfRequestSetCompletionRoutine(
            request,
            CompletionRoutine,
            CompletionContext
        );
    }

    if (WdfRequestSend(request,
        WdfUsbTargetDeviceGetIoTarget(UsbDevice),
        NULL) == FALSE)
    {
        return WdfRequestGetStatus(request);
    }

    return status;
}

void EvtUsbRequestCompletionRoutine(
    WDFREQUEST Request,
    WDFIOTARGET Target,
    PWDF_REQUEST_COMPLETION_PARAMS Params,
    WDFCONTEXT Context
)
{
    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Params);
    UNREFERENCED_PARAMETER(Context);

    WdfObjectDelete(Request);
}
