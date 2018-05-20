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
 
#pragma once


#include <stdlib.h>

#define BD_LINK_LENGTH  0x10

static const BYTE BD_LINK[BD_LINK_LENGTH] =
{
    0x56, 0xE8, 0x81, 0x38, 0x08, 0x06, 0x51, 0x41,
    0xC0, 0x7F, 0x12, 0xAA, 0xD9, 0x66, 0x3C, 0xCE
};

/**
* \typedef struct _BTH_HANDLE
*
* \brief   Defines a Bluetooth client handle.
*/
typedef struct _BTH_HANDLE
{
    BYTE Lsb;
    BYTE Msb;

} BTH_HANDLE, *PBTH_HANDLE;

/**
* \typedef struct _BTH_HANDLE_PAIR
*
* \brief   Defines a handle pair connecting a device CID to a host CID.
*/
typedef struct _BTH_HANDLE_PAIR
{
    BTH_HANDLE Source;
    BTH_HANDLE Destination;

} BTH_HANDLE_PAIR;

/**
* \typedef struct _BTH_DEVICE
*
* \brief   Defines a Bluetooth client device connection information set.
*/
typedef struct _BTH_DEVICE
{
    //
    // Handle identifying the parent host controller of this device
    // 
    BTH_HANDLE HCI_ConnectionHandle;

    //
    // Handle identifying the L2CAP command channel
    // 
    BTH_HANDLE_PAIR L2CAP_CommandHandle;

    //
    // Handle identifying the L2CAP interrupt channel
    // 
    BTH_HANDLE_PAIR L2CAP_InterruptHandle;

    //
    // Handle identifying the L2CAP service channel
    // 
    BTH_HANDLE_PAIR L2CAP_ServiceHandle;

    //
    // Indicates if the service channel can start
    // 
    BOOLEAN CanStartService;

    //
    // Indicates if the service channel is ready
    // 
    BOOLEAN IsServiceStarted;

    //
    // Indicates if the HID channel can start
    // 
    BOOLEAN CanStartHid;

    //
    // Index of the current HID initialization packet
    // 
    BYTE InitHidStage;

    //
    // Name reported by this device
    // 
    LPSTR RemoteName;

    //
    // Controller type
    // 
    BTH_DEVICE_TYPE DeviceType;

    //
    // Framework queue storing HID input requests
    // 
    WDFQUEUE HidInputReportQueue;

} BTH_DEVICE, *PBTH_DEVICE;

/**
* \typedef struct _BTH_DEVICE_LIST
*
* \brief   Defines a linked list of Bluetooth client devices.
*/
typedef struct _BTH_DEVICE_LIST
{
    ULONG logicalLength;

    PBTH_DEVICE head;

    PBTH_DEVICE tail;

} BTH_DEVICE_LIST, *PBTH_DEVICE_LIST;

#define BD_ADDR_FROM_BUFFER(_addr_, _buf_)      (RtlCopyMemory(&_addr_, _buf_, sizeof(BD_ADDR)));

/**
* \def BTH_HANDLE_FROM_BUFFER(_ch_, _buf_) (RtlCopyMemory(&_ch_, _buf_, sizeof(BTH_HANDLE)));
*
* \brief   A macro that extracts a BTH_HANDLE from a bulk input pipe buffer.
*
* \author  Benjamin "Nefarius" Höglinger
* \date    19.09.2017
*
* \param   _ch_    The target client handle.
* \param   _buf_   The buffer.
*/
#define BTH_HANDLE_FROM_BUFFER(_ch_, _buf_)     (RtlCopyMemory(&_ch_, _buf_, sizeof(BTH_HANDLE)));

/**
* \fn  VOID FORCEINLINE BTH_DEVICE_FREE( PBTH_DEVICE Device )
*
* \brief   Frees resources allocated by provided BTH_DEVICE.
*
* \author  Benjamin "Nefarius" Höglinger
* \date    20.09.2017
*
* \param   Device  The BTH_DEVICE handle.
*
* \return  Nothing.
*/
VOID FORCEINLINE BTH_DEVICE_FREE(
    PBTH_DEVICE Device
)
{
    if (Device->RemoteName)
        ExFreePoolWithTag(Device->RemoteName, WIRESHOCK_POOL_TAG);

    WdfIoQueuePurgeSynchronously(Device->HidInputReportQueue);
    WdfObjectDelete(Device->HidInputReportQueue);
}

typedef struct _PDO_IDENTIFICATION_DESCRIPTION 
{
    WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER Header;

    //
    // MAC address identifying this device
    // 
    BD_ADDR ClientAddress;

} PDO_IDENTIFICATION_DESCRIPTION, *PPDO_IDENTIFICATION_DESCRIPTION;

typedef struct _PDO_DEVICE_DATA
{
    //
    // MAC address identifying this device
    // 
    BD_ADDR ClientAddress;

    //
    // Child device state information
    // 
    BTH_DEVICE ClientDevice;

} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(PDO_DEVICE_DATA, PdoGetData)
