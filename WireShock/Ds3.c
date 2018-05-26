/*
MIT License

Copyright (c) 2017 Benjamin "Nefarius" Höglinger

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


#include "Driver.h"

#include "ds3.tmh"
#include "L2CAP.h"

NTSTATUS
Ds3ConnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PUCHAR CID)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONNECTION_REQUEST data = (PL2CAP_SIGNALLING_CONNECTION_REQUEST)&Buffer[8];

    scid = data->SCID;

    L2CAP_SET_CONNECTION_TYPE(
        Device,
        data->PSM,
        scid,
        &dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "! L2CAP_SET_CONNECTION_TYPE: PSM: %02X SCID: %04X DCID: %04X",
        data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Connection_Request PSM: %02X SCID: %04X DCID: %04X",
        data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Connection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid,
        dcid,
        L2CAP_ConnectionResponseResult_ConnectionPending
    );

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Connection_Response (PENDING) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Connection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid,
        dcid,
        L2CAP_ConnectionResponseResult_ConnectionSuccessful
    );

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Connection_Response (SUCCESSFUL) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Configuration_Request(
        Context,
        Device->HCI_ConnectionHandle,
        (*CID)++,
        scid,
        TRUE);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Configuration_Request failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3ConnectionResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PUCHAR CID)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONNECTION_RESPONSE data = (PL2CAP_SIGNALLING_CONNECTION_RESPONSE)&Buffer[8];

    UNREFERENCED_PARAMETER(CID);
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(Context);

    scid = data->SCID;
    dcid = data->DCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    switch ((L2CAP_CONNECTION_RESPONSE_RESULT)data->Result)
    {
    case L2CAP_ConnectionResponseResult_ConnectionSuccessful:

        /*
        L2CAP_SET_CONNECTION_TYPE(
            Device,
            L2CAP_PSM_HID_Service,
            dcid,
            &scid);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "! L2CAP_SET_CONNECTION_TYPE: L2CAP_PSM_HID_Service SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionSuccessful SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        status = L2CAP_Command_Configuration_Request(
            Context,
            Device->HCI_ConnectionHandle,
            (*CID)++,
            dcid,
            TRUE);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
                "L2CAP_Command_Configuration_Request failed");
            break;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< L2CAP_Configuration_Request SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);
            *
            */

        break;
    case L2CAP_ConnectionResponseResult_ConnectionPending:
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionPending");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable");
        break;
    default:
        break;
    }

    return status;
}

NTSTATUS Ds3ConfigurationRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;
    PVOID       pHidCmd;
    ULONG       hidCmdLen;

    PL2CAP_SIGNALLING_CONFIGURATION_REQUEST data = (PL2CAP_SIGNALLING_CONFIGURATION_REQUEST)&Buffer[8];

    dcid = data->DCID;

    L2CAP_DEVICE_GET_SCID(Device, dcid, &scid);


    L2CAP_CID   dcid_tmp;

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(Device, L2CAP_PSM_HID_Command, &dcid_tmp);
    if (RtlCompareMemory(&dcid, &dcid_tmp, sizeof(L2CAP_CID)) == sizeof(L2CAP_CID))
    {
        Device->IsHidCommandConfigured = TRUE;
    }

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(Device, L2CAP_PSM_HID_Interrupt, &dcid_tmp);
    if (RtlCompareMemory(&dcid, &dcid_tmp, sizeof(L2CAP_CID)) == sizeof(L2CAP_CID))
    {
        Device->IsHidInterruptConfigured = TRUE;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "! L2CAP_DEVICE_GET_SCID: DCID %04X -> SCID %04X",
        *(PUSHORT)&dcid, *(PUSHORT)&scid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid.Msb, *(PUSHORT)&dcid);

    status = L2CAP_Command_Configuration_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Configuration_Response failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Configuration_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    if (Device->IsHidCommandConfigured)
    {
        if (Device->InitHidStage < DS3_INIT_HID_STAGE_MAX)
        {
            L2CAP_DEVICE_GET_SCID_FOR_TYPE(
                Device,
                L2CAP_PSM_HID_Command,
                &scid);

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
                "!! L2CAP_DEVICE_GET_SCID_FOR_TYPE: L2CAP_PSM_HID_Command -> SCID %04X",
                *(PUSHORT)&scid);

            GetElementsByteArray(
                &Context->HidInitReports,
                Device->InitHidStage++,
                &pHidCmd,
                &hidCmdLen);

            status = HID_Command(
                Context,
                Device->HCI_ConnectionHandle,
                scid,
                pHidCmd,
                hidCmdLen);

            if (!NT_SUCCESS(status))
            {
                TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command failed");
                return status;
            }

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
                "<< HID_Command Index: %d, Length: %d",
                Device->InitHidStage - 1, hidCmdLen);
        }
    }

    return status;
}

NTSTATUS
Ds3ConfigurationResponse(
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE data = (PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE)&Buffer[8];

    scid = data->SCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Configuration_Response SCID: 0x%04X",
        *(PUSHORT)&scid);

    L2CAP_CID   dcid_tmp;

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(Device, L2CAP_PSM_HID_Command, &dcid_tmp);
    if (RtlCompareMemory(&scid, &dcid_tmp, sizeof(L2CAP_CID)) == sizeof(L2CAP_CID))
    {
        Device->IsHidCommandEstablished = TRUE;
    }

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(Device, L2CAP_PSM_HID_Interrupt, &dcid_tmp);
    if (RtlCompareMemory(&scid, &dcid_tmp, sizeof(L2CAP_CID)) == sizeof(L2CAP_CID))
    {
        Device->IsHidInterruptEstablished = TRUE;
    }

    return status;
}

NTSTATUS
Ds3DisconnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;
    L2CAP_CID   intDcid, comDcid;

    PL2CAP_SIGNALLING_DISCONNECTION_REQUEST data = (PL2CAP_SIGNALLING_DISCONNECTION_REQUEST)&Buffer[8];

    scid = data->SCID;
    dcid = data->DCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Disconnection_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(
        Device,
        L2CAP_PSM_HID_Interrupt,
        &intDcid);

    L2CAP_DEVICE_GET_DCID_FOR_TYPE(
        Device,
        L2CAP_PSM_HID_Command,
        &comDcid);

    if (*(PUSHORT)&intDcid == *(PUSHORT)&data->DCID
        || *(PUSHORT)&comDcid == *(PUSHORT)&data->DCID)
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "Invoking HCI_Command_Disconnect");

        status = HCI_Command_Disconnect(Context, Device->HCI_ConnectionHandle);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HCI_Command_Disconnect failed");
        }
    }

    status = L2CAP_Command_Disconnection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        dcid,
        scid);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Disconnection_Response failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Disconnection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3InitHidReportStage(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device)
{
    NTSTATUS                        status = STATUS_SUCCESS;
    L2CAP_CID                       scid;
    PVOID                           pHidCmd;
    ULONG                           hidCmdLen;

    if (Device->InitHidStage < DS3_INIT_HID_STAGE_MAX)
    {
        L2CAP_DEVICE_GET_SCID_FOR_TYPE(
            Device,
            L2CAP_PSM_HID_Command,
            &scid);

        GetElementsByteArray(
            &Context->HidInitReports,
            Device->InitHidStage++,
            &pHidCmd,
            &hidCmdLen);

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            pHidCmd,
            hidCmdLen);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< HID_Command Index: %d, Length: %d",
            Device->InitHidStage - 1, hidCmdLen);
    }
    else if (Device->IsHidCommandConfigured && !Device->IsHidInterruptEnabled)
    {
        BYTE hidCommandEnable[] = {
            0x53, 0xF4, 0x42, 0x03, 0x00, 0x00
        };
        BYTE hidOutputReport[] = {
            0x52, 0x01, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x1E, 0xFF, 0x27, 0x10, 0x00,
            0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF, 0x27,
            0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00, 0x32,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00
        };

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "Sending HID enable packet");

        L2CAP_DEVICE_GET_SCID_FOR_TYPE(
            Device,
            L2CAP_PSM_HID_Command,
            &scid);

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            hidCommandEnable,
            _countof(hidCommandEnable));

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command ENABLE failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< HID_Command ENABLE sent");

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            hidOutputReport,
            _countof(hidOutputReport));

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command OUTPUT REPORT failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< HID_Command OUTPUT REPORT sent");

        Device->IsHidInterruptEnabled = TRUE;
    }

    return status;
}

NTSTATUS
Ds3ProcessHidInputReport(
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status = STATUS_INVALID_PARAMETER;
    WDFREQUEST  Request;
    PUCHAR      outputBuffer;
    PUCHAR      inputBuffer;
    size_t      bufferLength;

    status = WdfIoQueueRetrieveNextRequest(Device->HidInputReportQueue, &Request);

    if (NT_SUCCESS(status))
    {
        status = WdfRequestRetrieveOutputBuffer(
            Request,
            DS3_HID_INPUT_REPORT_SIZE,
            (PVOID)&outputBuffer,
            &bufferLength);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
                "WdfRequestRetrieveOutputBuffer failed with status 0x%X (bufferLength: %d)",
                status, (ULONG)bufferLength);
            WdfRequestComplete(Request, status);
            return status;
        }

        // Shift to begin of report
        inputBuffer = &Buffer[9];

        // Report ID
        outputBuffer[0] = inputBuffer[0];

        // Prepare D-Pad
        outputBuffer[5] &= ~0xF; // Clear lower 4 bits

        // Translate D-Pad to HAT format
        switch (inputBuffer[2] & ~0xF)
        {
        case 0x10: // N
            outputBuffer[5] |= 0 & 0xF;
            break;
        case 0x30: // NE
            outputBuffer[5] |= 1 & 0xF;
            break;
        case 0x20: // E
            outputBuffer[5] |= 2 & 0xF;
            break;
        case 0x60: // SE
            outputBuffer[5] |= 3 & 0xF;
            break;
        case 0x40: // S
            outputBuffer[5] |= 4 & 0xF;
            break;
        case 0xC0: // SW
            outputBuffer[5] |= 5 & 0xF;
            break;
        case 0x80: // W
            outputBuffer[5] |= 6 & 0xF;
            break;
        case 0x90: // NW
            outputBuffer[5] |= 7 & 0xF;
            break;
        default: // Released
            outputBuffer[5] |= 8 & 0xF;
            break;
        }

        // Prepare face buttons
        outputBuffer[5] &= ~0xF0; // Clear upper 4 bits
        // Set face buttons
        outputBuffer[5] |= inputBuffer[3] & 0xF0;

        // Thumb axes
        outputBuffer[1] = inputBuffer[6]; // LTX
        outputBuffer[2] = inputBuffer[7]; // LTY
        outputBuffer[3] = inputBuffer[8]; // RTX
        outputBuffer[4] = inputBuffer[9]; // RTY

        // Remaining buttons
        outputBuffer[6] &= ~0xFF; // Clear all 8 bits
        outputBuffer[6] |= (inputBuffer[2] & 0xF);
        outputBuffer[6] |= (inputBuffer[3] & 0xF) << 4;

        // Trigger axes
        outputBuffer[8] = inputBuffer[18];
        outputBuffer[9] = inputBuffer[19];

        // PS button
        outputBuffer[7] = inputBuffer[4];

        // D-Pad (pressure)
        outputBuffer[10] = inputBuffer[14];
        outputBuffer[11] = inputBuffer[15];
        outputBuffer[12] = inputBuffer[16];
        outputBuffer[13] = inputBuffer[17];

        // Shoulders (pressure)
        outputBuffer[14] = inputBuffer[20];
        outputBuffer[15] = inputBuffer[21];

        // Face buttons (pressure)
        outputBuffer[16] = inputBuffer[22];
        outputBuffer[17] = inputBuffer[23];
        outputBuffer[18] = inputBuffer[24];
        outputBuffer[19] = inputBuffer[25];

        WdfRequestCompleteWithInformation(Request, status, bufferLength);
    }

    return status;
}
