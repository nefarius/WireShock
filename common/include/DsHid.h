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

#ifndef DsHid_h__
#define DsHid_h__

#include <ntddk.h>
#include <wdf.h>
#include <hidport.h>
#include "DsCommon.h"

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

extern CONST HID_REPORT_DESCRIPTOR G_Ds3HidReportDescriptor_Split_Mode[];

extern CONST HID_DESCRIPTOR G_Ds3HidDescriptor_Split_Mode;

extern CONST HID_REPORT_DESCRIPTOR G_Ds3HidReportDescriptor_Single_Mode[];

extern CONST HID_DESCRIPTOR G_Ds3HidDescriptor_Single_Mode;

#define DS3_HID_INPUT_REPORT_SIZE   0x27

VOID FORCEINLINE DS3_RAW_TO_SPLIT_HID_INPUT_REPORT_01(
    _In_ PUCHAR Input,
    _Out_ PUCHAR Output,
    _In_ BOOLEAN MuteDigitalPressureButtons
)
{
    // Report ID
    Output[0] = 0x01;

    // Prepare D-Pad
    Output[5] &= ~0xF; // Clear lower 4 bits

    // Prepare face buttons
    Output[5] &= ~0xF0; // Clear upper 4 bits

    // Remaining buttons
    Output[6] &= ~0xFF; // Clear all 8 bits

    if (!MuteDigitalPressureButtons)
    {
        // Translate D-Pad to HAT format
        switch (Input[2] & ~0xF)
        {
        case 0x10: // N
            Output[5] |= 0 & 0xF;
            break;
        case 0x30: // NE
            Output[5] |= 1 & 0xF;
            break;
        case 0x20: // E
            Output[5] |= 2 & 0xF;
            break;
        case 0x60: // SE
            Output[5] |= 3 & 0xF;
            break;
        case 0x40: // S
            Output[5] |= 4 & 0xF;
            break;
        case 0xC0: // SW
            Output[5] |= 5 & 0xF;
            break;
        case 0x80: // W
            Output[5] |= 6 & 0xF;
            break;
        case 0x90: // NW
            Output[5] |= 7 & 0xF;
            break;
        default: // Released
            Output[5] |= 8 & 0xF;
            break;
        }
        
        // Set face buttons
        Output[5] |= Input[3] & 0xF0;

        // Remaining buttons
        Output[6] |= (Input[2] & 0xF);
        Output[6] |= (Input[3] & 0xF) << 4;
    }
    else {
        // Clear HAT position
        Output[5] |= 8 & 0xF;
    }

    // Thumb axes
    Output[1] = Input[6]; // LTX
    Output[2] = Input[7]; // LTY
    Output[3] = Input[8]; // RTX
    Output[4] = Input[9]; // RTY

    // Trigger axes
    Output[8] = Input[18];
    Output[9] = Input[19];

    // PS button
    Output[7] = Input[4];

    // D-Pad (pressure)
    Output[10] = Input[14];
    Output[11] = Input[15];
    Output[12] = Input[16];
    Output[13] = Input[17];

    // Shoulders (pressure)
    Output[14] = Input[20];
    Output[15] = Input[21];

    // Face buttons (pressure)
    Output[16] = Input[22];
    Output[17] = Input[23];
    Output[18] = Input[24];
    Output[19] = Input[25];
}

VOID FORCEINLINE DS3_RAW_TO_SPLIT_HID_INPUT_REPORT_02(
    _In_ PUCHAR Input,
    _Out_ PUCHAR Output
)
{
    // Report ID
    Output[0] = 0x02;

    // D-Pad (pressure)
    Output[1] = Input[14];
    Output[2] = Input[15];
    Output[3] = Input[16];
    Output[4] = Input[17];

    // Face buttons (pressure)
    Output[5] = Input[22];
    Output[6] = Input[23];
    Output[7] = Input[24];
    Output[8] = Input[25];

    // Shoulders (pressure)
    // NOTE: not accessible via DirectInput because out axis limit
    Output[9] = Input[20];
    Output[10] = Input[21];
}

VOID FORCEINLINE DS3_RAW_TO_SINGLE_HID_INPUT_REPORT(
    _In_ PUCHAR Input,
    _Out_ PUCHAR Output,
    _In_ BOOLEAN MuteDigitalPressureButtons
)
{
    // Report ID
    Output[0] = Input[0];
    
    // Prepare D-Pad
    Output[5] &= ~0xF; // Clear lower 4 bits

    // Prepare face buttons
    Output[5] &= ~0xF0; // Clear upper 4 bits

    // Remaining buttons
    Output[6] &= ~0xFF; // Clear all 8 bits

    if (!MuteDigitalPressureButtons)
    {
        // Translate D-Pad to HAT format
        switch (Input[2] & ~0xF)
        {
        case 0x10: // N
            Output[5] |= 0 & 0xF;
            break;
        case 0x30: // NE
            Output[5] |= 1 & 0xF;
            break;
        case 0x20: // E
            Output[5] |= 2 & 0xF;
            break;
        case 0x60: // SE
            Output[5] |= 3 & 0xF;
            break;
        case 0x40: // S
            Output[5] |= 4 & 0xF;
            break;
        case 0xC0: // SW
            Output[5] |= 5 & 0xF;
            break;
        case 0x80: // W
            Output[5] |= 6 & 0xF;
            break;
        case 0x90: // NW
            Output[5] |= 7 & 0xF;
            break;
        default: // Released
            Output[5] |= 8 & 0xF;
            break;
        }
        
        // Set face buttons
        Output[5] |= Input[3] & 0xF0;

        // Remaining buttons
        Output[6] |= (Input[2] & 0xF);
        Output[6] |= (Input[3] & 0xF) << 4;
    }
    else {
        // Clear HAT position
        Output[5] |= 8 & 0xF;
    }

    // Thumb axes
    Output[1] = Input[6]; // LTX
    Output[2] = Input[7]; // LTY
    Output[3] = Input[8]; // RTX
    Output[4] = Input[9]; // RTY

    // Trigger axes
    Output[8] = Input[18];
    Output[9] = Input[19];

    // PS button
    Output[7] = Input[4];

    // D-Pad (pressure)
    Output[10] = Input[14];
    Output[11] = Input[15];
    Output[12] = Input[16];
    Output[13] = Input[17];

    // Shoulders (pressure)
    Output[14] = Input[20];
    Output[15] = Input[21];

    // Face buttons (pressure)
    Output[16] = Input[22];
    Output[17] = Input[23];
    Output[18] = Input[24];
    Output[19] = Input[25];
}

NTSTATUS
DsHidGetIndexedString(
    _In_  WDFREQUEST        Request,
    _In_  DS_DEVICE_TYPE    DsType
);

NTSTATUS
DsHidGetStringId(
    _In_  WDFREQUEST        Request,
    _Out_ ULONG            *StringId,
    _Out_ ULONG            *LanguageId
);

NTSTATUS
DsHidGetString(
    _In_  WDFREQUEST        Request,
    _In_  DS_DEVICE_TYPE    DsType
);

NTSTATUS
DsHidRequestCopyFromBuffer(
    _In_  WDFREQUEST        Request,
    _In_  PVOID             SourceBuffer,
    _When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
    _In_  size_t            NumBytesToCopyFrom
);

NTSTATUS
DsHidRequestGetHidXferPacket_ToReadFromDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET  *Packet
);

NTSTATUS
DsHidRequestGetHidXferPacket_ToWriteToDevice(
    _In_  WDFREQUEST        Request,
    _Out_ HID_XFER_PACKET  *Packet
);

NTSTATUS
DsHidGetDeviceDescriptor(
    _In_ WDFREQUEST         Request,
    _In_ DS_HID_DEVICE_MODE HidDeviceMode,
    _Inout_ size_t         *BytesCopied
);

NTSTATUS
DsHidGetReportDescriptor(
    _In_ WDFREQUEST         Request,
    _In_ DS_HID_DEVICE_MODE HidDeviceMode,
    _Inout_ size_t         *BytesCopied
);

#endif // DsHid_h__
