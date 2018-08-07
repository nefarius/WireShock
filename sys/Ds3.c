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
#include <DsHid.h>

NTSTATUS
Ds3ConnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PUCHAR CID)
{
    PL2CAP_SIGNALLING_CONNECTION_REQUEST data = (PL2CAP_SIGNALLING_CONNECTION_REQUEST)&Buffer[8];

    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid = data->SCID;

    L2CAP_SET_CONNECTION_TYPE(
        Device,
        data->PSM,
        scid,
        &dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        "! L2CAP_SET_CONNECTION_TYPE: PSM: %02X SCID: %04X DCID: %04X",
        data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_Command_Connection_Response (PENDING) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_Command_Connection_Response (SUCCESSFUL) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_Command_Configuration_Request failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        "<< L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3ConnectionResponse(
    PUCHAR Buffer)
{
    PL2CAP_SIGNALLING_CONNECTION_RESPONSE data = (PL2CAP_SIGNALLING_CONNECTION_RESPONSE)&Buffer[8];

    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   dcid = data->DCID;
    L2CAP_CID   scid = data->SCID;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        ">> L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    switch ((L2CAP_CONNECTION_RESPONSE_RESULT)data->Result)
    {
    case L2CAP_ConnectionResponseResult_ConnectionSuccessful:
        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionSuccessful");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionPending:
        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionPending");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable");
        break;
    default:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "Unknown L2CAP_CONNECTION_RESPONSE_RESULT %d, Default case triggered",
            (L2CAP_CONNECTION_RESPONSE_RESULT)data->Result);
        break;
    }

    return status;
}

NTSTATUS Ds3ConfigurationRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    PL2CAP_SIGNALLING_CONFIGURATION_REQUEST data = (PL2CAP_SIGNALLING_CONFIGURATION_REQUEST)&Buffer[8];

    NTSTATUS    status;
    L2CAP_CID   dcid = data->DCID;
    L2CAP_CID   scid;

    L2CAP_DEVICE_GET_SCID(Device, dcid, &scid);

    L2CAP_CID   dcid_tmp; //Temporary, right?

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

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        "! L2CAP_DEVICE_GET_SCID: DCID %04X -> SCID %04X",
        *(PUSHORT)&dcid, *(PUSHORT)&scid);

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        ">> L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid.Msb, *(PUSHORT)&dcid);

    status = L2CAP_Command_Configuration_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_Command_Configuration_Response failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        "<< L2CAP_Configuration_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3ConfigurationResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE data = (PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE)&Buffer[8];

    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   scid = data->SCID;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        ">> L2CAP_Configuration_Response SCID: 0x%04X",
        *(PUSHORT)&scid);

    L2CAP_CID   dcid_tmp; //Temporary, right?

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

    if (Device->IsHidCommandEstablished && Device->IsHidInterruptEstablished)
    {
        BYTE hidCommandEnable[] = {
            0x53, 0xF4, 0x42, 0x03, 0x00, 0x00
        };

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
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
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DS3,
                "HID_Command ENABLE failed with status %!STATUS!",
                status);
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
            "<< HID_Command ENABLE sent");

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            (PVOID)G_Ds3HidOutputReport,
            DS3_HID_OUTPUT_REPORT_SIZE);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DS3,
                "HID_Command OUTPUT REPORT failed with status %!STATUS!",
                status);
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
            "<< HID_Command OUTPUT REPORT sent");

        WdfTimerStart(
            Device->OutputReportTimer,
            WDF_REL_TIMEOUT_IN_MS(DS_ORT_PERIODIC_DUE_TIME)
        );
    }

    return status;
}

NTSTATUS
Ds3DisconnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    PL2CAP_SIGNALLING_DISCONNECTION_REQUEST data = (PL2CAP_SIGNALLING_DISCONNECTION_REQUEST)&Buffer[8];

    NTSTATUS    status;
    L2CAP_CID   dcid = data->DCID;
    L2CAP_CID   scid = data->SCID;
    L2CAP_CID   intDcid, comDcid;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
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
        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_DS3,
            "Invoking HCI_Command_Disconnect");

        status = HCI_Command_Disconnect(Context, Device->HCI_ConnectionHandle);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_DS3,
                "HCI_Command_Disconnect failed with status %!STATUS!",
                status);
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
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DS3,
            "L2CAP_Command_Disconnection_Response failed with status %!STATUS!",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_DS3,
        "<< L2CAP_Disconnection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

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

    // Shift to the beginning of the report
    inputBuffer = &Buffer[9];

#pragma region HID Input Report (ID 01) processing

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

        // Convert input to report
        DS3_RAW_TO_HID_INPUT_REPORT_01(inputBuffer, outputBuffer);

        WdfRequestCompleteWithInformation(Request, status, bufferLength);
    }

#pragma endregion

#pragma region HID Input Report (ID 02) processing

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

        // Convert input to report
        DS3_RAW_TO_HID_INPUT_REPORT_02(inputBuffer, outputBuffer);

        WdfRequestCompleteWithInformation(Request, status, bufferLength);
    }

#pragma endregion

    return status;
}
