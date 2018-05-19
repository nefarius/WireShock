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

DEFINE_GUID (GUID_DEVINTERFACE_WireShock,
    0x747b893a,0x709f,0x48ae,0x9a,0x74,0x9c,0x99,0xf1,0x82,0x4f,0x77);
// {747b893a-709f-48ae-9a74-9c99f1824f77}


#pragma once

/**
* \typedef struct _BD_ADDR
*
* \brief   Defines a Bluetooth client MAC address.
*/
typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR, *PBD_ADDR;

/**
* \typedef enum _BTH_DEVICE_TYPE
*
* \brief   Defines an alias representing the possible types of the BTH_DEVICE.
*/
typedef enum _BTH_DEVICE_TYPE
{
    DsTypeUnknown,
    DualShock3,
    DualShock4

} BTH_DEVICE_TYPE;
