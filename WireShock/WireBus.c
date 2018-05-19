#include "Driver.h"

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
    
    return STATUS_SUCCESS;
}