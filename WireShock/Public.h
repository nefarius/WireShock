/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_WIRESHOCK,
    0x747b893a,0x709f,0x48ae,0x9a,0x74,0x9c,0x99,0xf1,0x82,0x4f,0x77);
// {747b893a-709f-48ae-9a74-9c99f1824f77}


#pragma once

