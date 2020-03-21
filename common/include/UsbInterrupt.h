#ifndef UsbInterrupt_h__
#define UsbInterrupt_h__

#include <ntddk.h>
#include <wdf.h>
#include <usb.h>
#include <wdfusb.h>

#define USB_INTERRUPT_POOL_TAG      'TPIU'

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
    _In_opt_ __drv_aliasesMem WDFCONTEXT        CompletionContext);

EVT_WDF_REQUEST_COMPLETION_ROUTINE EvtUsbRequestCompletionRoutine;

#endif // UsbInterrupt_h__
