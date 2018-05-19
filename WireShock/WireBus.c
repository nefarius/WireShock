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
    UNREFERENCED_PARAMETER(ChildList);
    UNREFERENCED_PARAMETER(ChildInit);
    UNREFERENCED_PARAMETER(IdentificationDescription);

    PPDO_IDENTIFICATION_DESCRIPTION pDesc;

    PAGED_CODE();

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

    return STATUS_SUCCESS;
}