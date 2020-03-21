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

#include "DsHid.h"
// Utilize target project tracing
#include "../../sys/Trace.h"
#include "DsHid.tmh"

#pragma region DS3 HID Report Descriptor (Split Device Mode)

CONST HID_REPORT_DESCRIPTOR G_Ds3HidReportDescriptor_Split_Mode[] = {
    /************************************************************************/
    /* Gamepad definition (for regular DS3 buttons, axes & features)        */
    /************************************************************************/
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0x01,        //     Report ID (1)
    0x06, 0x01, 0xFF,  //     Usage Page (Vendor Defined 0xFF01)
    0x09, 0x01,        //     Usage (0x01)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x1D,        //     Report Count (29)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_HOST_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC0,        //     Report ID (192)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_SET_HOST_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC1,        //     Report ID (193)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC2,        //     Report ID (194)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_TYPE
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC3,        //     Report ID (195)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_CONNECTION_TYPE
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC4,        //     Report ID (196)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_CONFIG
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC5,        //     Report ID (197)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x07,        //     Report Count (7)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_SET_DEVICE_CONFIG
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC6,        //     Report ID (198)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x07,        //     Report Count (7)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
   /************************************************************************/
   /* Joystick definition (for exposing pressure values as axes)           */
   /************************************************************************/
   0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
   0x15, 0x00,        // Logical Minimum (0)
   0x09, 0x04,        // Usage (Joystick)
   0xA1, 0x01,        // Collection (Application)
   0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
   0x85, 0x02,        //   Report ID (2)
   0x09, 0x01,        //   Usage (Pointer)
   0x15, 0x00,        //   Logical Minimum (0)
   0x26, 0xFF, 0x00,  //   Logical Maximum (255)
   0x75, 0x08,        //   Report Size (8)
   0x95, 0x01,        //   Report Count (1)
   0xA1, 0x00,        //   Collection (Physical)
   0x09, 0x30,        //     Usage (X)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x31,        //     Usage (Y)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x32,        //     Usage (Z)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x33,        //     Usage (Rx)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x34,        //     Usage (Ry)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x35,        //     Usage (Rz)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x36,        //     Usage (Slider)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0x37,        //     Usage (Dial)
   0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0xC0,              //   End Collection
   0x05, 0x02,        //   Usage Page (Sim Ctrls)
   0x09, 0xBB,        //   Usage (Throttle)
   0x15, 0x00,        //   Logical Minimum (0)
   0x26, 0xFF, 0x00,  //   Logical Maximum (255)
   0x75, 0x08,        //   Report Size (8)
   0x95, 0x01,        //   Report Count (1)
   0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0x09, 0xC5,        //   Usage (Brake)
   0x15, 0x00,        //   Logical Minimum (0)
   0x26, 0xFF, 0x00,  //   Logical Maximum (255)
   0x75, 0x08,        //   Report Size (8)
   0x95, 0x01,        //   Report Count (1)
   0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
   0xC0,              // End Collection
};

CONST HID_DESCRIPTOR G_Ds3HidDescriptor_Split_Mode = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
{ 0x22,   // descriptor type 
sizeof(G_Ds3HidReportDescriptor_Split_Mode) }  // total length of report descriptor
};

#pragma endregion

#pragma region DS3 HID Report Descriptor (Single Device Mode)

CONST HID_REPORT_DESCRIPTOR G_Ds3HidReportDescriptor_Single_Mode[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0xA1, 0x00,        //   Collection (Physical)
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xA1, 0x02,        //     Collection (Logical)
    0x09, 0x36,        //       Usage (Slider)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //     End Collection
    0xC0,              //   End Collection
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0x01,        //     Report ID (1)
    0x06, 0x01, 0xFF,  //     Usage Page (Vendor Defined 0xFF01)
    0x09, 0x01,        //     Usage (0x01)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x1D,        //     Report Count (29)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_HOST_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC0,        //     Report ID (192)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_SET_HOST_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC1,        //     Report ID (193)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_BD_ADDR
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC2,        //     Report ID (194)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x08,        //     Report Count (8)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_TYPE
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC3,        //     Report ID (195)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_CONNECTION_TYPE
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC4,        //     Report ID (196)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_GET_DEVICE_CONFIG
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC5,        //     Report ID (197)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x07,        //     Report Count (7)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    //
    // DS_FEATURE_TYPE_SET_DEVICE_CONFIG
    // 
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0xC6,        //     Report ID (198)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x07,        //     Report Count (7)
    0x09, 0x01,        //     Usage (0x01)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

CONST HID_DESCRIPTOR G_Ds3HidDescriptor_Single_Mode = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
{ 0x22,   // descriptor type 
sizeof(G_Ds3HidReportDescriptor_Single_Mode) }  // total length of report descriptor
};

#pragma endregion

NTSTATUS DsHidGetIndexedString(WDFREQUEST Request, DS_DEVICE_TYPE DsType)
{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    ULONG                   languageId, stringIndex;

    status = DsHidGetStringId(Request, &stringIndex, &languageId);

    // While we don't use the language id, some minidrivers might.
    //
    UNREFERENCED_PARAMETER(languageId);

    if (NT_SUCCESS(status)) {

        switch (DsType)
        {
        case DS_DEVICE_TYPE_PS3_DUALSHOCK:
            if (stringIndex != DS3_DEVICE_STRING_INDEX)
            {
                status = STATUS_INVALID_PARAMETER;
                TraceEvents(TRACE_LEVEL_ERROR,
                    TRACE_DSHID,
                    "%!FUNC!: unknown string index %d",
                    stringIndex);
                return status;
            }

            status = DsHidRequestCopyFromBuffer(Request, DS3_DEVICE_STRING, sizeof(DS3_DEVICE_STRING));
            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DSHID,
                "%!FUNC!: Unknown DsType %d, Default case triggered",
                DsType);
            break;
        }
    }
    return status;
}

NTSTATUS DsHidGetStringId(WDFREQUEST Request, ULONG * StringId, ULONG * LanguageId)
{
    NTSTATUS                status;
    ULONG                   inputValue;
    WDF_REQUEST_PARAMETERS  requestParameters;

    //
    // IOCTL_HID_GET_STRING:                      // METHOD_NEITHER
    // IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT
    //
    // The string id (or string index) is passed in Parameters.DeviceIoControl.
    // Type3InputBuffer. However, Parameters.DeviceIoControl.InputBufferLength
    // was not initialized by hidclass.sys, therefore trying to access the
    // buffer with WdfRequestRetrieveInputMemory will fail
    //
    // Another problem with IOCTL_HID_GET_INDEXED_STRING is that METHOD_OUT_DIRECT
    // expects the input buffer to be Irp->AssociatedIrp.SystemBuffer instead of
    // Type3InputBuffer. That will also fail WdfRequestRetrieveInputMemory.
    //
    // The solution to the above two problems is to get Type3InputBuffer directly
    //
    // Also note that instead of the buffer's content, it is the buffer address
    // that was used to store the string id (or index)
    //

    WDF_REQUEST_PARAMETERS_INIT(&requestParameters);
    WdfRequestGetParameters(Request, &requestParameters);

    inputValue = PtrToUlong(
        requestParameters.Parameters.DeviceIoControl.Type3InputBuffer);

    status = STATUS_SUCCESS;

    //
    // The least significant two bytes of the INT value contain the string id.
    //
    *StringId = (inputValue & 0x0ffff);

    //
    // The most significant two bytes of the INT value contain the language
    // ID (for example, a value of 1033 indicates English).
    //
    *LanguageId = (inputValue >> 16);

    return status;
}

NTSTATUS DsHidGetString(WDFREQUEST Request, DS_DEVICE_TYPE DsType)
{
    NTSTATUS                status;
    ULONG                   languageId, stringId;
    size_t                  stringSizeCb = 0;
    PWSTR                   string = NULL;

    status = DsHidGetStringId(Request, &stringId, &languageId);

    // While we don't use the language id, some minidrivers might.
    //
    UNREFERENCED_PARAMETER(languageId);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    switch (stringId) {
    case HID_STRING_ID_IMANUFACTURER:

        stringSizeCb = sizeof(DSX_MANUFACTURER_STRING);
        string = DSX_MANUFACTURER_STRING;

        break;
    case HID_STRING_ID_IPRODUCT:
        switch (DsType)
        {
        case DS_DEVICE_TYPE_PS3_DUALSHOCK:
            stringSizeCb = sizeof(DS3_PRODUCT_STRING);
            string = DS3_PRODUCT_STRING;
            break;
        case DS_DEVICE_TYPE_PS3_NAVIGATION:
            stringSizeCb = sizeof(PS_MOVE_NAVIGATION_PRODUCT_STRING);
            string = PS_MOVE_NAVIGATION_PRODUCT_STRING;
            break;
        case DS_DEVICE_TYPE_PS3_MOTION:
            stringSizeCb = sizeof(PS_MOVE_MOTION_PRODUCT_STRING);
            string = PS_MOVE_MOTION_PRODUCT_STRING;
            break;
        default:
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DSHID,
                "%!FUNC!: Unknown DsType %d, Default case triggered",
                DsType);
            break;
        }
        break;
        /*
        case HID_STRING_ID_ISERIALNUMBER:
        switch (DsType)
        {
        case DualShock3:
        stringSizeCb = sizeof(VHIDMINI_SERIAL_NUMBER_STRING);
        string = VHIDMINI_SERIAL_NUMBER_STRING;
        break;
        default:
        break;
        }
        break;
        */
    default:
        status = STATUS_INVALID_PARAMETER;
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "%!FUNC!: unknown string id %d",
            stringId);
        return status;
    }

    return DsHidRequestCopyFromBuffer(Request, string, stringSizeCb);
}

NTSTATUS DsHidRequestCopyFromBuffer(WDFREQUEST Request, PVOID SourceBuffer, size_t NumBytesToCopyFrom)
{
    NTSTATUS                status = STATUS_INVALID_PARAMETER;
    WDFMEMORY               memory;
    size_t                  outputBufferLength;

    if (SourceBuffer == NULL) {
        return status;
    }

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "WdfRequestRetrieveOutputMemory failed with status %!STATUS!",
            status);
        return status;
    }

    WdfMemoryGetBuffer(memory, &outputBufferLength);
    if (outputBufferLength < NumBytesToCopyFrom) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "%!FUNC!: buffer too small. Got %d, expected %d",
            (int)outputBufferLength, (int)NumBytesToCopyFrom);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    status = WdfMemoryCopyFromBuffer(memory,
        0,
        SourceBuffer,
        NumBytesToCopyFrom);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "WdfMemoryCopyFromBuffer failed with status %!STATUS!",
            status);
        return status;
    }

    WdfRequestSetInformation(Request, NumBytesToCopyFrom);
    return status;
}

//
// First let's review Buffer Descriptions for I/O Control Codes
//
//   METHOD_BUFFERED
//    - Input buffer:  Irp->AssociatedIrp.SystemBuffer
//    - Output buffer: Irp->AssociatedIrp.SystemBuffer
//
//   METHOD_IN_DIRECT or METHOD_OUT_DIRECT
//    - Input buffer:  Irp->AssociatedIrp.SystemBuffer
//    - Second buffer: Irp->MdlAddress
//
//   METHOD_NEITHER
//    - Input buffer:  Parameters.DeviceIoControl.Type3InputBuffer;
//    - Output buffer: Irp->UserBuffer
//
// HID minidriver IOCTL stores a pointer to HID_XFER_PACKET in Irp->UserBuffer.
// For IOCTLs like IOCTL_HID_GET_FEATURE (which is METHOD_OUT_DIRECT) this is
// not the expected buffer location. So we cannot retrieve UserBuffer from the
// IRP using WdfRequestXxx functions. Instead, we have to escape to WDM.
//

NTSTATUS
DsHidRequestGetHidXferPacket_ToReadFromDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET  *Packet
)
{
    WDF_REQUEST_PARAMETERS  params;

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_XFER_PACKET)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "%!FUNC!: invalid HID_XFER_PACKET");
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(Packet, WdfRequestWdmGetIrp(Request)->UserBuffer, sizeof(HID_XFER_PACKET));
    return STATUS_SUCCESS;
}

NTSTATUS
DsHidRequestGetHidXferPacket_ToWriteToDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET  *Packet
)
{
    WDF_REQUEST_PARAMETERS  params;

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "%!FUNC!: invalid HID_XFER_PACKET");
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(Packet, WdfRequestWdmGetIrp(Request)->UserBuffer, sizeof(HID_XFER_PACKET));
    return STATUS_SUCCESS;
}

NTSTATUS DsHidGetDeviceDescriptor(
    WDFREQUEST Request,
    DS_HID_DEVICE_MODE HidDeviceMode,
    size_t *BytesCopied
)
{
    NTSTATUS        status;
    WDFMEMORY       memory;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DSHID,
        ">> IOCTL_HID_GET_DEVICE_DESCRIPTOR");

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "WdfRequestRetrieveOutputMemory failed with status %!STATUS!",
            status);
        return status;
    }

    switch (HidDeviceMode)
    {
    case DS_HID_DEVICE_MODE_MULTI:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DSHID,
            "Using multi-mode device descriptor");

        *BytesCopied = G_Ds3HidDescriptor_Split_Mode.bLength;

        status = WdfMemoryCopyFromBuffer(memory,
            0, // Offset
            (PVOID)&G_Ds3HidDescriptor_Split_Mode,
            *BytesCopied);

        break;
    case DS_HID_DEVICE_MODE_SINGLE:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DSHID,
            "Using single-mode device descriptor");

        *BytesCopied = G_Ds3HidDescriptor_Single_Mode.bLength;

        status = WdfMemoryCopyFromBuffer(memory,
            0, // Offset
            (PVOID)&G_Ds3HidDescriptor_Single_Mode,
            *BytesCopied);

        break;
    default:
        break;
    }

    return status;
}

NTSTATUS DsHidGetReportDescriptor(
    WDFREQUEST Request,
    DS_HID_DEVICE_MODE HidDeviceMode,
    size_t *BytesCopied)
{
    NTSTATUS        status;
    WDFMEMORY       memory;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DSHID,
        ">> IOCTL_HID_GET_REPORT_DESCRIPTOR");

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DSHID,
            "WdfRequestRetrieveOutputMemory failed with status %!STATUS!",
            status);
        return status;
    }

    switch (HidDeviceMode)
    {
    case DS_HID_DEVICE_MODE_MULTI:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DSHID,
            "Using multi-mode device descriptor");

        *BytesCopied = G_Ds3HidDescriptor_Split_Mode.DescriptorList[0].wReportLength;

        if (*BytesCopied == 0) {
            status = STATUS_INVALID_DEVICE_STATE;
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DSHID,
                "G_Ds3HidDescriptor_Split_Mode's reportLenght is zero, %!STATUS!",
                status);
            break;
        }

        status = WdfMemoryCopyFromBuffer(memory,
            0,
            (PVOID)G_Ds3HidReportDescriptor_Split_Mode,
            *BytesCopied);

        break;
    case DS_HID_DEVICE_MODE_SINGLE:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DSHID,
            "Using single-mode device descriptor");

        *BytesCopied = G_Ds3HidDescriptor_Single_Mode.DescriptorList[0].wReportLength;

        if (*BytesCopied == 0) {
            status = STATUS_INVALID_DEVICE_STATE;
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DSHID,
                "G_Ds3HidDescriptor_Single_Mode's reportLenght is zero, %!STATUS!",
                status);
            break;
        }

        status = WdfMemoryCopyFromBuffer(memory,
            0,
            (PVOID)G_Ds3HidReportDescriptor_Single_Mode,
            *BytesCopied);

        break;
    default:
        break;
    }

    return status;
}
