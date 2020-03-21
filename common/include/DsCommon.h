#ifndef DsCommon_h__
#define DsCommon_h__

#define DSX_MANUFACTURER_STRING             L"Nefarius Software Solutions"
#define DS3_PRODUCT_STRING                  L"DualShock 3 Controller"
#define DS3_DEVICE_STRING_INDEX             5
#define DS3_DEVICE_STRING                   L"U.N. Owen" // TODO: where's this displayed?
#define PS_MOVE_NAVIGATION_PRODUCT_STRING   L"Navigation Controller"
#define PS_MOVE_MOTION_PRODUCT_STRING       L"Motion Controller"

/**
* \typedef struct _BD_ADDR
*
* \brief   Defines a Bluetooth client MAC address.
*/
typedef struct _BD_ADDR
{
    UCHAR Address[6];

} BD_ADDR, *PBD_ADDR;

/**
 * \enum    _DS_DEVICE_TYPE
 *
 * \brief   Device type.
 */
typedef enum _DS_DEVICE_TYPE
{
    //
    // Unknown device type
    // 
    DS_DEVICE_TYPE_UNKNOWN,

    //
    // Sony DualShock 3 Controller
    // 
    DS_DEVICE_TYPE_PS3_DUALSHOCK,

    //
    // Sony DualShock 4 Controller
    // 
    DS_DEVICE_TYPE_PS4_DUALSHOCK,

    //
    // Sony Navigation Controller
    // 
    DS_DEVICE_TYPE_PS3_NAVIGATION,

    //
    // Sony Motion Controller
    // 
    DS_DEVICE_TYPE_PS3_MOTION

} DS_DEVICE_TYPE, *PDS_DEVICE_TYPE;

/**
 * \enum    _DS_CONNECTION_TYPE
 *
 * \brief   Device connection type.
 */
typedef enum _DS_CONNECTION_TYPE
{
    //
    // Unknown connection type
    // 
    DS_CONNECTION_TYPE_UNKNOWN,

    //
    // Connected via USB (wired)
    // 
    DS_CONNECTION_TYPE_USB,

    //
    // Connected via Bluetooth (wireless)
    // 
    DS_CONNECTION_TYPE_BLUETOOTH

} DS_CONNECTION_TYPE, *PDS_CONNECTION_TYPE;

/**
 * \enum    _DS_FEATURE_TYPE
 *
 * \brief   Supported feature requests. These values must match the IDs in the report descriptor.
 */
typedef enum _DS_FEATURE_TYPE
{
    //
    // Receive controller-embedded Bluetooth host address
    // 
    DS_FEATURE_TYPE_GET_HOST_BD_ADDR = 0xC0,

    //
    // Update controller-embedded Bluetooth host address
    // 
    DS_FEATURE_TYPE_SET_HOST_BD_ADDR = 0xC1,

    //
    // Receive controller-embedded Bluetooth address
    // 
    DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR = 0xC2,

    //
    // Receive device type (DS3, DS4, ...)
    // 
    DS_FEATURE_TYPE_GET_DEVICE_TYPE = 0xC3,

    //
    // Receive device connection type (USB, Bluetooth)
    // 
    DS_FEATURE_TYPE_GET_CONNECTION_TYPE = 0xC4,

    //
    // Receive current volatile configuration properties
    // 
    DS_FEATURE_TYPE_GET_DEVICE_CONFIG = 0xC5,

    //
    // Update current volatile configuration properties
    // 
    DS_FEATURE_TYPE_SET_DEVICE_CONFIG = 0xC6

} DS_FEATURE_TYPE, *PDS_FEATURE_TYPE;

/**
 * \enum    _DS_HID_DEVICE_MODE
 *
 * \brief   HID device mode (influences used HID Report Descriptor).
 */
typedef enum _DS_HID_DEVICE_MODE
{
    DS_HID_DEVICE_MODE_SINGLE,

    DS_HID_DEVICE_MODE_MULTI

} DS_HID_DEVICE_MODE, *PDS_HID_DEVICE_MODE;

#include <pshpack1.h>

/**
 * \struct  _DS_DRIVER_CONFIGURATION
 *
 * \brief   Drive configuration properties.
 *
 * \author  Benjamin "Nefarius" Höglinger-Stelzer
 * \date    06.08.2018
 */
typedef struct _DS_DRIVER_CONFIGURATION
{
    DS_HID_DEVICE_MODE HidDeviceMode;

    BOOLEAN MuteDigitalPressureButtons;

} DS_DRIVER_CONFIGURATION, *PDS_DRIVER_CONFIGURATION;

typedef struct _DS_FEATURE_HEADER
{
    UCHAR ReportId;

    ULONG Size;

} DS_FEATURE_HEADER, *PDS_FEATURE_HEADER;

#pragma region DS_FEATURE_GET_HOST_BD_ADDR

typedef struct _DS_FEATURE_GET_HOST_BD_ADDR
{
    DS_FEATURE_HEADER Header;

    BD_ADDR HostAddress;

} DS_FEATURE_GET_HOST_BD_ADDR, *PDS_FEATURE_GET_HOST_BD_ADDR;

VOID FORCEINLINE DS_FEATURE_GET_HOST_BD_ADDR_INIT(
    PDS_FEATURE_GET_HOST_BD_ADDR Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_GET_HOST_BD_ADDR));
    Request->Header.ReportId = DS_FEATURE_TYPE_GET_HOST_BD_ADDR;
    Request->Header.Size = sizeof(DS_FEATURE_GET_HOST_BD_ADDR);
}

#pragma endregion

#pragma region DS_FEATURE_SET_HOST_BD_ADDR

typedef struct _DS_FEATURE_SET_HOST_BD_ADDR
{
    DS_FEATURE_HEADER Header;

    BD_ADDR HostAddress;

} DS_FEATURE_SET_HOST_BD_ADDR, *PDS_FEATURE_SET_HOST_BD_ADDR;

VOID FORCEINLINE DS_FEATURE_SET_HOST_BD_ADDR_INIT(
    PDS_FEATURE_SET_HOST_BD_ADDR Request,
    BD_ADDR HostAddress
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_SET_HOST_BD_ADDR));
    Request->Header.ReportId = DS_FEATURE_TYPE_SET_HOST_BD_ADDR;
    Request->Header.Size = sizeof(DS_FEATURE_SET_HOST_BD_ADDR);
    Request->HostAddress = HostAddress;
}

#pragma endregion

#pragma region DS_FEATURE_GET_DEVICE_BD_ADDR

typedef struct _DS_FEATURE_GET_DEVICE_BD_ADDR
{
    DS_FEATURE_HEADER Header;

    BD_ADDR DeviceAddress;

} DS_FEATURE_GET_DEVICE_BD_ADDR, *PDS_FEATURE_GET_DEVICE_BD_ADDR;

VOID FORCEINLINE DS_FEATURE_GET_DEVICE_BD_ADDR_INIT(
    PDS_FEATURE_GET_DEVICE_BD_ADDR Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_GET_DEVICE_BD_ADDR));
    Request->Header.ReportId = DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR;
    Request->Header.Size = sizeof(DS_FEATURE_GET_DEVICE_BD_ADDR);
}

#pragma endregion

#pragma region DS_FEATURE_GET_DEVICE_TYPE

typedef struct _DS_FEATURE_GET_DEVICE_TYPE
{
    DS_FEATURE_HEADER Header;

    DS_DEVICE_TYPE DeviceType;

} DS_FEATURE_GET_DEVICE_TYPE, *PDS_FEATURE_GET_DEVICE_TYPE;

VOID FORCEINLINE DS_FEATURE_GET_DEVICE_TYPE_INIT(
    PDS_FEATURE_GET_DEVICE_TYPE Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_GET_DEVICE_TYPE));
    Request->Header.ReportId = DS_FEATURE_TYPE_GET_DEVICE_TYPE;
    Request->Header.Size = sizeof(DS_FEATURE_GET_DEVICE_TYPE);
}

#pragma endregion

#pragma region DS_FEATURE_GET_CONNECTION_TYPE

typedef struct _DS_FEATURE_GET_CONNECTION_TYPE
{
    DS_FEATURE_HEADER Header;

    DS_CONNECTION_TYPE ConnectionType;

} DS_FEATURE_GET_CONNECTION_TYPE, *PDS_FEATURE_GET_CONNECTION_TYPE;

VOID FORCEINLINE DS_FEATURE_GET_CONNECTION_TYPE_INIT(
    PDS_FEATURE_GET_CONNECTION_TYPE Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_GET_CONNECTION_TYPE));
    Request->Header.ReportId = DS_FEATURE_TYPE_GET_CONNECTION_TYPE;
    Request->Header.Size = sizeof(DS_FEATURE_GET_CONNECTION_TYPE);
}

#pragma endregion

#pragma region DS_FEATURE_GET_DEVICE_CONFIG

typedef struct _DS_FEATURE_GET_DEVICE_CONFIG
{
    DS_FEATURE_HEADER Header;

    DS_DRIVER_CONFIGURATION Configuration;

} DS_FEATURE_GET_DEVICE_CONFIG, *PDS_FEATURE_GET_DEVICE_CONFIG;

VOID FORCEINLINE DS_FEATURE_GET_DEVICE_CONFIG_INIT(
    PDS_FEATURE_GET_DEVICE_CONFIG Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_GET_DEVICE_CONFIG));
    Request->Header.ReportId = DS_FEATURE_TYPE_GET_DEVICE_CONFIG;
    Request->Header.Size = sizeof(DS_FEATURE_GET_DEVICE_CONFIG);
}

#pragma endregion

#pragma region DS_FEATURE_SET_DEVICE_CONFIG

typedef struct _DS_FEATURE_SET_DEVICE_CONFIG
{
    DS_FEATURE_HEADER Header;

    DS_DRIVER_CONFIGURATION Configuration;

} DS_FEATURE_SET_DEVICE_CONFIG, *PDS_FEATURE_SET_DEVICE_CONFIG;

VOID FORCEINLINE DS_FEATURE_SET_DEVICE_CONFIG_INIT(
    PDS_FEATURE_SET_DEVICE_CONFIG Request
)
{
    RtlZeroMemory(Request, sizeof(DS_FEATURE_SET_DEVICE_CONFIG));
    Request->Header.ReportId = DS_FEATURE_TYPE_SET_DEVICE_CONFIG;
    Request->Header.Size = sizeof(DS_FEATURE_SET_DEVICE_CONFIG);
}

#pragma endregion

#include <poppack.h>

#endif // DsCommon_h__
