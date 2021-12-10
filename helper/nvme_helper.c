//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2012-2021 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
// 
#if !defined(DISABLE_NVME_PASSTHROUGH)

#include "platform_helper.h"
#include <stdio.h>
#include "nvme_helper.h"
#include "nvme_helper_func.h"
#include "common_public.h"


// \file nvme_cmds.c   Implementation for NVM Express helper functions
//                     The intention of the file is to be generic & not OS specific

// \fn fill_In_NVMe_Device_Info(device device)
// \brief Sends a set Identify etc commands & fills in the device information
// \param device device struture
// \return SUCCESS - pass, !SUCCESS fail or something went wrong
int fill_In_NVMe_Device_Info(tDevice *device)
{
    int ret = UNKNOWN;

    //set some pointers to where we want to fill in information...we're doing this so that on USB, we can store some info about the child drive, without disrupting the standard drive_info that has already been filled in by the fill_SCSI_Info function
    char *fillModelNumber = device->drive_info.product_identification;
    char *fillSerialNumber = device->drive_info.serialNumber;
    char *fillFWRev = device->drive_info.product_revision;
    uint64_t *fillWWN = &device->drive_info.worldWideName;
    uint32_t *fillLogicalSectorSize = &device->drive_info.deviceBlockSize;
    uint32_t *fillPhysicalSectorSize = &device->drive_info.devicePhyBlockSize;
    uint16_t *fillSectorAlignment = &device->drive_info.sectorAlignment;
    uint64_t *fillMaxLba = &device->drive_info.deviceMaxLba;

    //If not an NVMe interface, such as USB, then we need to store things differently
    //RAID Interface should be treated as "Native" or "NVME_INTERFACE" since there is likely an underlying API providing direct access of some kind.
    if (device->drive_info.interface_type != NVME_INTERFACE && device->drive_info.interface_type != RAID_INTERFACE)
    {
        device->drive_info.bridge_info.isValid = true;
        fillModelNumber = device->drive_info.bridge_info.childDriveMN;
        fillSerialNumber = device->drive_info.bridge_info.childDriveSN;
        fillFWRev = device->drive_info.bridge_info.childDriveFW;
        fillWWN = &device->drive_info.bridge_info.childWWN;
        fillLogicalSectorSize = &device->drive_info.bridge_info.childDeviceBlockSize;
        fillPhysicalSectorSize = &device->drive_info.bridge_info.childDevicePhyBlockSize;
        fillSectorAlignment = &device->drive_info.bridge_info.childSectorAlignment;
        fillMaxLba = &device->drive_info.bridge_info.childDeviceMaxLba;
    }

    nvmeIDCtrl * ctrlData = &device->drive_info.IdentifyData.nvme.ctrl; //Conroller information data structure
    nvmeIDNameSpaces * nsData = &device->drive_info.IdentifyData.nvme.ns; //Name Space Data structure 
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif

    ret = nvme_Identify(device, C_CAST(uint8_t *, ctrlData), 0, NVME_IDENTIFY_CTRL);

#ifdef _DEBUG
printf("fill NVMe info ret = %d\n", ret);
#endif

    if (ret == SUCCESS)
    {
        //set the t10 vendor id to NVMe
        snprintf(device->drive_info.T10_vendor_ident, T10_VENDOR_ID_LEN + 1, "NVMe");
        device->drive_info.media_type = MEDIA_NVM;//This will bite us someday when someone decided to put non-ssds on NVMe interface.

        //Set the other device fields we need.
        memcpy(fillSerialNumber,ctrlData->sn,SERIAL_NUM_LEN);
        fillSerialNumber[20] = '\0';
        remove_Leading_And_Trailing_Whitespace(fillSerialNumber);
        memcpy(fillFWRev, ctrlData->fr,8); //8 is the NVMe spec length of this
        fillFWRev[8] = '\0';
        remove_Leading_And_Trailing_Whitespace(fillFWRev);
        memcpy(fillModelNumber, ctrlData->mn,MODEL_NUM_LEN); 
        fillModelNumber[40] = '\0';
        remove_Leading_And_Trailing_Whitespace(fillModelNumber);
        //Do not overwrite this with non-NVMe interfaces. This is used by USB to figure out and track bridge chip specific things that are stored in this location
        if (device->drive_info.interface_type == NVME_INTERFACE && !device->drive_info.adapter_info.vendorIDValid)
        {
            device->drive_info.adapter_info.vendorID = ctrlData->vid;
            device->drive_info.adapter_info.vendorIDValid = true;
        }
        //set the IEEE OUI into the WWN since we use the WWN for detecting if the drive is a Seagate drive.
        //TODO: currently we set NAA to 5, but we should probably at least follow the SCSI-NVMe translation specification!
        *fillWWN = M_BytesTo8ByteValue(0x05, ctrlData->ieee[2], ctrlData->ieee[1], ctrlData->ieee[0], 0, 0, 0, 0) << 4;

        ret = nvme_Identify(device, C_CAST(uint8_t *, nsData), device->drive_info.namespaceID, NVME_IDENTIFY_NS);

        if (ret == SUCCESS) 
        {

            *fillLogicalSectorSize = C_CAST(uint32_t, power_Of_Two(nsData->lbaf[nsData->flbas].lbaDS)); //removed math.h pow() function - TJE
            *fillPhysicalSectorSize = *fillLogicalSectorSize; //True for NVMe?
            *fillSectorAlignment = 0;

            *fillMaxLba = nsData->nsze - 1;//spec says this is from 0 to (n-1)!
            

            //TODO: Add support if more than one Namespace. 
            /*
            for (ns=2; ns <= ctrlData.nn; ns++) 
            {
                

            }*/

        }
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif

    return ret;
}

void print_NVMe_Cmd_Verbose(const nvmeCmdCtx * cmdCtx)
{
    printf("Sending NVM Command:\n");
    printf("\tType: ");
    switch (cmdCtx->commandType)
    {
    case NVM_ADMIN_CMD:
        printf("Admin");
        break;
    case NVM_CMD:
        printf("NVM");
        break;
    case NVM_UNKNOWN_CMD_SET:
    default:
        printf("Unknown");
        break;
    }
    printf("\n");
    printf("\tData Direction: ");
    //Data Direction:
    switch (cmdCtx->commandDirection)
    {
    case XFER_NO_DATA:
        printf("No Data");
        break;
    case XFER_DATA_IN:
        printf("Data In");
        break;
    case XFER_DATA_OUT:
        printf("Data Out");
        break;
    default:
        printf("Unknown");
        break;
    }
    printf("\n");
    printf("Data Length: %" PRIu32 "\n", cmdCtx->dataSize);
    //printf("Cmd result 0x%02X\n", cmdCtx->result);
    printf("Command Bytes:\n");
    switch (cmdCtx->commandType)
    {
    case NVM_ADMIN_CMD:
        printf("\tOpcode (CDW0) = %" PRIu8 "\n", cmdCtx->cmd.adminCmd.opcode);
        printf("\tFlags (CDW0) = %" PRIu8 "\n", cmdCtx->cmd.adminCmd.flags);
        printf("\tReserved (CDW0) = %" PRIu16 "\n", cmdCtx->cmd.adminCmd.rsvd1);
        printf("\tNSID = %" PRIX32 "h\n", cmdCtx->cmd.adminCmd.nsid);
        printf("\tCDW2 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw2);
        printf("\tCDW3 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw3);
        printf("\tMetadata Ptr = %" PRIX64 "h\n", cmdCtx->cmd.adminCmd.metadata);
        printf("\tMetadata Length = %" PRIu32 "\n", cmdCtx->cmd.adminCmd.metadataLen);
        printf("\tData Ptr = %" PRIX64 "h\n", cmdCtx->cmd.adminCmd.addr);
        printf("\tCDW10 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw10);
        printf("\tCDW11 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw11);
        printf("\tCDW12 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw12);
        printf("\tCDW13 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw13);
        printf("\tCDW14 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw14);
        printf("\tCDW15 = %08" PRIX32 "h\n", cmdCtx->cmd.adminCmd.cdw15);
        break;
    case NVM_CMD:
        printf("\tOpcode (CDW0) = %" PRIu8 "\n", cmdCtx->cmd.nvmCmd.opcode);
        printf("\tFlags (CDW0) = %" PRIu8 "\n", cmdCtx->cmd.nvmCmd.flags);
        printf("\tCommand ID (CDW0) = %" PRIu16 "\n", cmdCtx->cmd.nvmCmd.commandId);
        printf("\tNSID = %" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.nsid);
        printf("\tCDW2 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw2);
        printf("\tCDW3 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw3);
        printf("\tMetadata Ptr (CDW4 & 5) = %" PRIX64 "h\n", cmdCtx->cmd.nvmCmd.metadata);
        printf("\tData Pointer (CDW6 & 7) = %" PRIX64 "h\n", cmdCtx->cmd.nvmCmd.prp1);
        printf("\tData Pointer (CDW8 & 9) = %" PRIX64 "h\n", cmdCtx->cmd.nvmCmd.prp2);
        printf("\tCDW10 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw10);
        printf("\tCDW11 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw11);
        printf("\tCDW12 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw12);
        printf("\tCDW13 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw13);
        printf("\tCDW14 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw14);
        printf("\tCDW15 = %08" PRIX32 "h\n", cmdCtx->cmd.nvmCmd.cdw15);
        break;
    default:
        printf("\tCDW0  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw0);
        printf("\tCDW1  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw1);
        printf("\tCDW2  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw2);
        printf("\tCDW3  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw3);
        printf("\tCDW4  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw4);
        printf("\tCDW5  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw5);
        printf("\tCDW6  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw6);
        printf("\tCDW7  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw7);
        printf("\tCDW8  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw8);
        printf("\tCDW9  = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw9);
        printf("\tCDW10 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw10);
        printf("\tCDW11 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw11);
        printf("\tCDW12 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw12);
        printf("\tCDW13 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw13);
        printf("\tCDW14 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw14);
        printf("\tCDW15 = %08" PRIX32 "h\n", cmdCtx->cmd.dwords.cdw15);
        break;
    }
    printf("\n");
}

void get_NVMe_Status_Fields_From_DWord(uint32_t nvmeStatusDWord, bool *doNotRetry, bool *more, uint8_t *statusCodeType, uint8_t *statusCode)
{
    if (doNotRetry && more && statusCodeType && statusCode)
    {
        *doNotRetry = nvmeStatusDWord & BIT31;
        *more  = nvmeStatusDWord & BIT30;
        *statusCodeType = M_GETBITRANGE(nvmeStatusDWord, 27, 25);
        *statusCode = M_GETBITRANGE(nvmeStatusDWord, 24, 17);
    }
}

//TODO: this function needs to be expanded as new status codes are added
//TODO: use doNotRetry and more bits in some useful way?
int check_NVMe_Status(uint32_t nvmeStatusDWord)
{
    int ret = SUCCESS;
    //bool doNotRetry = nvmeStatusDWord & BIT31;
    //bool more  = nvmeStatusDWord & BIT30;
    uint8_t statusCodeType = M_GETBITRANGE(nvmeStatusDWord, 27, 25);
    uint8_t statusCode = M_GETBITRANGE(nvmeStatusDWord, 24, 17);

    switch (statusCodeType)
    {
    case NVME_SCT_GENERIC_COMMAND_STATUS://generic
        switch (statusCode)
        {
        case NVME_GEN_SC_SUCCESS_:
            ret = SUCCESS;
            break;
        case NVME_GEN_SC_INVALID_OPCODE_:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_INVALID_FIELD_:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_CMDID_CONFLICT_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_DATA_XFER_ERROR_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_POWER_LOSS_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_INTERNAL_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_ABORT_REQ_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_ABORT_QUEUE_:
            ret = ABORTED;
            break;
        case NVME_GEN_SC_FUSED_FAIL_:
            ret = ABORTED;
            break;
        case NVME_GEN_SC_FUSED_MISSING_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_INVALID_NS_:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_CMD_SEQ_ERROR_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_INVALID_SGL_SEGMENT_DESCRIPTOR:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_INVALID_NUMBER_OF_SGL_DESCRIPTORS:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_DATA_SGL_LENGTH_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_METADATA_SGL_LENGTH_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_SGL_DESCRIPTOR_TYPE_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_INVALID_USE_OF_CONTROLLER_MEMORY_BUFFER:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_PRP_OFFSET_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_ATOMIC_WRITE_UNIT_EXCEEDED:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_OPERATION_DENIED:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_SGL_OFFSET_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_HOST_IDENTIFIER_INCONSISTENT_FORMAT:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_KEEP_ALIVE_TIMEOUT_EXPIRED:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_KEEP_ALIVE_TIMEOUT_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_COMMAND_ABORTED_DUE_TO_PREEMPT_AND_ABORT:
            ret = ABORTED;
            break;
        case NVME_GEN_SC_SANITIZE_FAILED:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_SANITIZE_IN_PROGRESS:
            ret = IN_PROGRESS;
            break;
        case NVME_GEN_SC_SGL_DATA_BLOCK_GRANULARITY_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_GEN_SC_COMMAND_NOT_SUPPORTED_FOR_QUEUE_IN_CMB:
            ret = NOT_SUPPORTED;
            break;
            //80-BF are NVM command set specific                
        case NVME_GEN_SC_LBA_RANGE_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_CAP_EXCEEDED_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_NS_NOT_READY_:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_RESERVATION_CONFLICT:
            ret = FAILURE;
            break;
        case NVME_GEN_SC_FORMAT_IN_PROGRESS:
            ret = IN_PROGRESS;
            break;
        default:
            ret = UNKNOWN;
            break;
        }
        break;
    case NVME_SCT_COMMAND_SPECIFIC_STATUS://command specific
        switch (statusCode)
        {
        case NVME_CMD_SP_SC_CQ_INVALID_:
        case NVME_CMD_SP_SC_QID_INVALID_:
        case NVME_CMD_SP_SC_QUEUE_SIZE_:
        case NVME_CMD_SP_SC_ABORT_LIMIT_:
            //NVME_CMD_SP_SC_ABORT_MISSING_ = 0x04,//reserved in NVMe specs
        case NVME_CMD_SP_SC_ASYNC_LIMIT_:
            ret = FAILURE;
            break;
        case NVME_CMD_SP_SC_INVALID_FIRMWARE_SLOT_:
        case NVME_CMD_SP_SC_INVALIDFIRMWARE_IMAGE_:
        case NVME_CMD_SP_SC_INVALID_INTERRUPT_VECTOR_:
        case NVME_CMD_SP_SC_INVALID_LOG_PAGE_:
        case NVME_CMD_SP_SC_INVALID_FORMAT_:
            ret = NOT_SUPPORTED;
            break;
        case NVME_CMD_SP_SC_FW_ACT_REQ_CONVENTIONAL_RESET:
            ret = SUCCESS;
            break;
        case NVME_CMD_SP_SC_INVALID_QUEUE_DELETION:
        case NVME_CMD_SP_SC_FEATURE_IDENTIFIER_NOT_SAVABLE:
        case NVME_CMD_SP_SC_FEATURE_NOT_CHANGEABLE:
        case NVME_CMD_SP_SC_FEATURE_NOT_NAMESPACE_SPECIFC:
            ret = FAILURE;
            break;
        case NVME_CMD_SP_SC_FW_ACT_REQ_NVM_SUBSYS_RESET:
        case NVME_CMD_SP_SC_FW_ACT_REQ_RESET:
        case NVME_CMD_SP_SC_FW_ACT_REQ_MAX_TIME_VIOALTION:
            ret = SUCCESS;
            break;
        case NVME_CMD_SP_SC_FW_ACT_PROHIBITED:
        case NVME_CMD_SP_SC_OVERLAPPING_RANGE:
        case NVME_CMD_SP_SC_NS_INSUFFICIENT_CAP:
        case NVME_CMD_SP_SC_NS_ID_UNAVAILABLE:
        case NVME_CMD_SP_SC_NS_ALREADY_ATTACHED:
        case NVME_CMD_SP_SC_NS_IS_PRIVATE:
        case NVME_CMD_SP_SC_NS_NOT_ATTACHED:
            ret = FAILURE;
            break;
        case NVME_CMD_SP_SC_THIN_PROVISIONING_NOT_SUPPORTED:
        case NVME_CMD_SP_SC_CONTROLLER_LIST_INVALID:
            ret = NOT_SUPPORTED;
            break;
        case NVME_CMD_SP_SC_DEVICE_SELF_TEST_IN_PROGRESS:
            ret = IN_PROGRESS;
            break;
        case NVME_CMD_SP_SC_BOOT_PARTITION_WRITE_PROHIBITED:
            ret = FAILURE;
            break;
        case NVME_CMD_SP_SC_INVALID_CONTROLLER_IDENTIFIER:
        case NVME_CMD_SP_SC_INVALID_SECONDARY_CONTROLLER_STATE:
        case NVME_CMD_SP_SC_INVALID_NUMBER_OF_CONTROLLER_RESOURCES:
        case NVME_CMD_SP_SC_INVALID_RESOURCE_IDENTIFIER:
            ret = NOT_SUPPORTED;
            break;
            //80-BF are NVM command set specific                 
        case NVME_CMD_SP_SC_CONFLICTING_ATTRIBUTES_:
        case NVME_CMD_SP_SC_INVALID_PROTECTION_INFORMATION:
        case NVME_CMD_SP_SC_ATTEMPTED_WRITE_TO_READ_ONLY_RANGE:
            ret = FAILURE;
            break;
        default:
            ret = UNKNOWN;
            break;
        }
        break;
    case NVME_SCT_MEDIA_AND_DATA_INTEGRITY_ERRORS://media or data errors
        switch (statusCode)
        {
        case NVME_MED_ERR_SC_WRITE_FAULT_:
        case NVME_MED_ERR_SC_UNREC_READ_ERROR_:
        case NVME_MED_ERR_SC_ETE_GUARD_CHECK_:
        case NVME_MED_ERR_SC_ETE_APPTAG_CHECK_:
        case NVME_MED_ERR_SC_ETE_REFTAG_CHECK_:
        case NVME_MED_ERR_SC_COMPARE_FAILED_:
        case NVME_MED_ERR_SC_ACCESS_DENIED_:
        case NVME_MED_ERR_SC_DEALLOCATED_OR_UNWRITTEN_LOGICAL_BLOCK:
            ret = FAILURE;
            break;
        default:
            ret = UNKNOWN;
            break;
        }
        break;
    case NVME_SCT_VENDOR_SPECIFIC_STATUS:
        //fall through to default.
    default:
        //unknown meaning. Either reserved or vendor unique.
        ret = UNKNOWN;
        break;
    }
    return ret;
}

void print_NVMe_Cmd_Result_Verbose(const nvmeCmdCtx * cmdCtx)
{
    //TODO: Print out the result/error information!
    printf("NVM Command Completion:\n");
    printf("\tCommand Specific (DW0): ");
    if (cmdCtx->commandCompletionData.dw0Valid)
    {
        printf("%" PRIX32 "h\n", cmdCtx->commandCompletionData.commandSpecific);
    }
    else
    {
        printf("Unavailable from OS\n");
    }
    printf("\tReserved (DW1): ");
    if (cmdCtx->commandCompletionData.dw1Valid)
    {
        printf("%" PRIX32 "h\n", cmdCtx->commandCompletionData.dw1Reserved);
    }
    else
    {
        printf("Unavailable from OS\n");
    }
    printf("\tSQ ID & SQ Head Ptr (DW2): ");
    if (cmdCtx->commandCompletionData.dw2Valid)
    {
        printf("%" PRIX32 "h\n", cmdCtx->commandCompletionData.sqIDandHeadPtr);
    }
    else
    {
        printf("Unavailable from OS\n");
    }
    printf("\tStatus & CID (DW3): ");
    if (cmdCtx->commandCompletionData.dw3Valid)
    {
        bool dnr = false, more = false;
        uint8_t statusCodeType = 0;
        uint8_t statusCode = 0;
        printf("%" PRIX32 "h\n", cmdCtx->commandCompletionData.statusAndCID);
        get_NVMe_Status_Fields_From_DWord(cmdCtx->commandCompletionData.statusAndCID, &dnr, &more, &statusCodeType, &statusCode);
        printf("\t\tDo Not Retry: ");
        if (dnr)
        {
            printf("True\n");
        }
        else
        {
            printf("False\n");
        }
        printf("\t\tMore: ");
        if (more)
        {
            printf("True\n");
        }
        else
        {
            printf("False\n");
        }
        char statusCodeTypeString[60] = { 0 };
        char statusCodeString[60] = { 0 };
        //also print out the phase tag, CID. NOTE: These aren't available in Linux!
        switch (statusCodeType)
        {
        case NVME_SCT_GENERIC_COMMAND_STATUS://generic
            snprintf(statusCodeTypeString, 60, "Generic Status");
            switch (statusCode)
            {
            case NVME_GEN_SC_SUCCESS_:
                snprintf(statusCodeString, 60, "Success");
                break;
            case NVME_GEN_SC_INVALID_OPCODE_:
                snprintf(statusCodeString, 60, "Invalid Command Opcode");
                break;
            case NVME_GEN_SC_INVALID_FIELD_:
                snprintf(statusCodeString, 60, "Invalid Field in Command");
                break;
            case NVME_GEN_SC_CMDID_CONFLICT_:
                snprintf(statusCodeString, 60, "Command ID Conflict");
                break;
            case NVME_GEN_SC_DATA_XFER_ERROR_:
                snprintf(statusCodeString, 60, "Data Transfer Error");
                break;
            case NVME_GEN_SC_POWER_LOSS_:
                snprintf(statusCodeString, 60, "Commands Aborted due to Power Less Notification");
                break;
            case NVME_GEN_SC_INTERNAL_:
                snprintf(statusCodeString, 60, "Internal Error");
                break;
            case NVME_GEN_SC_ABORT_REQ_:
                snprintf(statusCodeString, 60, "Command Abort Requested");
                break;
            case NVME_GEN_SC_ABORT_QUEUE_:
                snprintf(statusCodeString, 60, "Command Aborted due to SQ Deletion");
                break;
            case NVME_GEN_SC_FUSED_FAIL_:
                snprintf(statusCodeString, 60, "Command Aborted due to Failed Fused Command");
                break;
            case NVME_GEN_SC_FUSED_MISSING_:
                snprintf(statusCodeString, 60, "Command Aborted due to Missing Fused Command");
                break;
            case NVME_GEN_SC_INVALID_NS_:
                snprintf(statusCodeString, 60, "Invalid Namespace or Format");
                break;
            case NVME_GEN_SC_CMD_SEQ_ERROR_:
                snprintf(statusCodeString, 60, "Command Sequence Error");
                break;
            case NVME_GEN_SC_INVALID_SGL_SEGMENT_DESCRIPTOR:
                snprintf(statusCodeString, 60, "Invalid SGL Segment Descriptor");
                break;
            case NVME_GEN_SC_INVALID_NUMBER_OF_SGL_DESCRIPTORS:
                snprintf(statusCodeString, 60, "Invalid Number of SGL Descriptors");
                break;
            case NVME_GEN_SC_DATA_SGL_LENGTH_INVALID:
                snprintf(statusCodeString, 60, "Data SGL Length Invalid");
                break;
            case NVME_GEN_SC_METADATA_SGL_LENGTH_INVALID:
                snprintf(statusCodeString, 60, "Metadata SGL Length Invalid");
                break;
            case NVME_GEN_SC_SGL_DESCRIPTOR_TYPE_INVALID:
                snprintf(statusCodeString, 60, "SGL Descriptor Type Invalid");
                break;
            case NVME_GEN_SC_INVALID_USE_OF_CONTROLLER_MEMORY_BUFFER:
                snprintf(statusCodeString, 60, "Invalid Use of Controller Memory Buffer");
                break;
            case NVME_GEN_SC_PRP_OFFSET_INVALID:
                snprintf(statusCodeString, 60, "PRP Offset Invalid");
                break;
            case NVME_GEN_SC_ATOMIC_WRITE_UNIT_EXCEEDED:
                snprintf(statusCodeString, 60, "Atomic Write Unit Exceeded");
                break;
            case NVME_GEN_SC_OPERATION_DENIED:
                snprintf(statusCodeString, 60, "Operation Denied");
                break;
            case NVME_GEN_SC_SGL_OFFSET_INVALID:
                snprintf(statusCodeString, 60, "SGL Offset Invalid");
                break;
            case NVME_GEN_SC_HOST_IDENTIFIER_INCONSISTENT_FORMAT:
                snprintf(statusCodeString, 60, "Host Identifier Inconsistent Format");
                break;
            case NVME_GEN_SC_KEEP_ALIVE_TIMEOUT_EXPIRED:
                snprintf(statusCodeString, 60, "Keep Alive Timeout Expired");
                break;
            case NVME_GEN_SC_KEEP_ALIVE_TIMEOUT_INVALID:
                snprintf(statusCodeString, 60, "Keel Alive Timeout Invalid");
                break;
            case NVME_GEN_SC_COMMAND_ABORTED_DUE_TO_PREEMPT_AND_ABORT:
                snprintf(statusCodeString, 60, "Command Aborted due to Preempt and Abort");
                break;
            case NVME_GEN_SC_SANITIZE_FAILED:
                snprintf(statusCodeString, 60, "Sanitize Failed");
                break;
            case NVME_GEN_SC_SANITIZE_IN_PROGRESS:
                snprintf(statusCodeString, 60, "Sanitize In Progress");
                break;
            case NVME_GEN_SC_SGL_DATA_BLOCK_GRANULARITY_INVALID:
                snprintf(statusCodeString, 60, "SGL Data Block Granularity Invalid");
                break;
            case NVME_GEN_SC_COMMAND_NOT_SUPPORTED_FOR_QUEUE_IN_CMB:
                snprintf(statusCodeString, 60, "Command Not Supported for Queue in CMB");
                break;
                //80-BF are NVM command set specific                
            case NVME_GEN_SC_LBA_RANGE_:
                snprintf(statusCodeString, 60, "LBA Out of Range");
                break;
            case NVME_GEN_SC_CAP_EXCEEDED_:
                snprintf(statusCodeString, 60, "Capacity Exceeded");
                break;
            case NVME_GEN_SC_NS_NOT_READY_:
                snprintf(statusCodeString, 60, "Namespace Not Ready");
                break;
            case NVME_GEN_SC_RESERVATION_CONFLICT:
                snprintf(statusCodeString, 60, "Reservation Conflict");
                break;
            case NVME_GEN_SC_FORMAT_IN_PROGRESS:
                snprintf(statusCodeString, 60, "Format In Progress");
                break;
            default:
                snprintf(statusCodeString, 60, "Unknown");
                break;
            }
            break;
        case NVME_SCT_COMMAND_SPECIFIC_STATUS://command specific
            snprintf(statusCodeTypeString, 60, "Command Specific Status");
            switch (statusCode)
            {
            case NVME_CMD_SP_SC_CQ_INVALID_:
                snprintf(statusCodeString, 60, "Completion Queue Invalid");
                break;
            case NVME_CMD_SP_SC_QID_INVALID_:
                snprintf(statusCodeString, 60, "Invalid Queue Identifier");
                break;
            case NVME_CMD_SP_SC_QUEUE_SIZE_:
                snprintf(statusCodeString, 60, "Invalid Queue Size");
                break;
            case NVME_CMD_SP_SC_ABORT_LIMIT_:
                snprintf(statusCodeString, 60, "Aborted Command Limit Exceeded");
                break;
                //NVME_CMD_SP_SC_ABORT_MISSING_ = 0x04,//reserved in NVMe specs
            case NVME_CMD_SP_SC_ASYNC_LIMIT_:
                snprintf(statusCodeString, 60, "Asynchronous Event Request Limit Exceeded");
                break;
            case NVME_CMD_SP_SC_INVALID_FIRMWARE_SLOT_:
                snprintf(statusCodeString, 60, "Invalid Firmware Slot");
                break;
            case NVME_CMD_SP_SC_INVALIDFIRMWARE_IMAGE_:
                snprintf(statusCodeString, 60, "Invalid Firmware Image");
                break;
            case NVME_CMD_SP_SC_INVALID_INTERRUPT_VECTOR_:
                snprintf(statusCodeString, 60, "Invalid Interrupt Vector");
                break;
            case NVME_CMD_SP_SC_INVALID_LOG_PAGE_:
                snprintf(statusCodeString, 60, "Invalid Log Page");
                break;
            case NVME_CMD_SP_SC_INVALID_FORMAT_:
                snprintf(statusCodeString, 60, "Invalid Format");
                break;
            case NVME_CMD_SP_SC_FW_ACT_REQ_CONVENTIONAL_RESET:
                snprintf(statusCodeString, 60, "Firmware Activation Requires Conventional Reset");
                break;
            case NVME_CMD_SP_SC_INVALID_QUEUE_DELETION:
                snprintf(statusCodeString, 60, "Invalid Queue Deletion");
                break;
            case NVME_CMD_SP_SC_FEATURE_IDENTIFIER_NOT_SAVABLE:
                snprintf(statusCodeString, 60, "Feature Identifier Not Savable");
                break;
            case NVME_CMD_SP_SC_FEATURE_NOT_CHANGEABLE:
                snprintf(statusCodeString, 60, "Feature Not Changeable");
                break;
            case NVME_CMD_SP_SC_FEATURE_NOT_NAMESPACE_SPECIFC:
                snprintf(statusCodeString, 60, "Feature Not Namespace Specific");
                break;
            case NVME_CMD_SP_SC_FW_ACT_REQ_NVM_SUBSYS_RESET:
                snprintf(statusCodeString, 60, "Firmware Activation Requires NVM Subsystem Reset");
                break;
            case NVME_CMD_SP_SC_FW_ACT_REQ_RESET:
                snprintf(statusCodeString, 60, "Firmware Activation Requires Reset");
                break;
            case NVME_CMD_SP_SC_FW_ACT_REQ_MAX_TIME_VIOALTION:
                snprintf(statusCodeString, 60, "Firmware Activation Requires Maximum Time Violation");
                break;
            case NVME_CMD_SP_SC_FW_ACT_PROHIBITED:
                snprintf(statusCodeString, 60, "Firmware Activation Prohibited");
                break;
            case NVME_CMD_SP_SC_OVERLAPPING_RANGE:
                snprintf(statusCodeString, 60, "Overlapping Range");
                break;
            case NVME_CMD_SP_SC_NS_INSUFFICIENT_CAP:
                snprintf(statusCodeString, 60, "Namespace Insufficient Capacity");
                break;
            case NVME_CMD_SP_SC_NS_ID_UNAVAILABLE:
                snprintf(statusCodeString, 60, "Namespace Identifier Unavailable");
                break;
            case NVME_CMD_SP_SC_NS_ALREADY_ATTACHED:
                snprintf(statusCodeString, 60, "Namespace Already Attached");
                break;
            case NVME_CMD_SP_SC_NS_IS_PRIVATE:
                snprintf(statusCodeString, 60, "Namespace Is Private");
                break;
            case NVME_CMD_SP_SC_NS_NOT_ATTACHED:
                snprintf(statusCodeString, 60, "Namespace Not Attached");
                break;
            case NVME_CMD_SP_SC_THIN_PROVISIONING_NOT_SUPPORTED:
                snprintf(statusCodeString, 60, "Thin Provisioning Not Supported");
                break;
            case NVME_CMD_SP_SC_CONTROLLER_LIST_INVALID:
                snprintf(statusCodeString, 60, "Controller List Invalid");
                break;
            case NVME_CMD_SP_SC_DEVICE_SELF_TEST_IN_PROGRESS:
                snprintf(statusCodeString, 60, "Device Self-test In Progress");
                break;
            case NVME_CMD_SP_SC_BOOT_PARTITION_WRITE_PROHIBITED:
                snprintf(statusCodeString, 60, "Boot Partition Write Prohibited");
                break;
            case NVME_CMD_SP_SC_INVALID_CONTROLLER_IDENTIFIER:
                snprintf(statusCodeString, 60, "Invalid Controller Identifier");
                break;
            case NVME_CMD_SP_SC_INVALID_SECONDARY_CONTROLLER_STATE:
                snprintf(statusCodeString, 60, "Invalid Secondary Controller State");
                break;
            case NVME_CMD_SP_SC_INVALID_NUMBER_OF_CONTROLLER_RESOURCES:
                snprintf(statusCodeString, 60, "Invalid Number of Controller Resources");
                break;
            case NVME_CMD_SP_SC_INVALID_RESOURCE_IDENTIFIER:
                snprintf(statusCodeString, 60, "Invalid Resource Identifier");
                break;
                //80-BF are NVM command set specific                 
            case NVME_CMD_SP_SC_CONFLICTING_ATTRIBUTES_:
                snprintf(statusCodeString, 60, "Conflicting Attributes");
                break;
            case NVME_CMD_SP_SC_INVALID_PROTECTION_INFORMATION:
                snprintf(statusCodeString, 60, "Invalid Protection Information");
                break;
            case NVME_CMD_SP_SC_ATTEMPTED_WRITE_TO_READ_ONLY_RANGE:
                snprintf(statusCodeString, 60, "Attempted Write to Read Only Range");
                break;
            default:
                snprintf(statusCodeString, 60, "Unknown");
                break;
            }
            break;
        case NVME_SCT_MEDIA_AND_DATA_INTEGRITY_ERRORS://media or data errors
            snprintf(statusCodeTypeString, 60, "Media And Data Integrity Errors");
            switch (statusCode)
            {
            case NVME_MED_ERR_SC_WRITE_FAULT_:
                snprintf(statusCodeString, 60, "Write Fault");
                break;
            case NVME_MED_ERR_SC_UNREC_READ_ERROR_:
                snprintf(statusCodeString, 60, "Unrecovered Read Error");
                break;
            case NVME_MED_ERR_SC_ETE_GUARD_CHECK_:
                snprintf(statusCodeString, 60, "End-to-end Guard Check Error");
                break;
            case NVME_MED_ERR_SC_ETE_APPTAG_CHECK_:
                snprintf(statusCodeString, 60, "End-to-end Application Tag Check Error");
                break;
            case NVME_MED_ERR_SC_ETE_REFTAG_CHECK_:
                snprintf(statusCodeString, 60, "End-to-end Reference Tag Check Error");
                break;
            case NVME_MED_ERR_SC_COMPARE_FAILED_:
                snprintf(statusCodeString, 60, "Compare Failure");
                break;
            case NVME_MED_ERR_SC_ACCESS_DENIED_:
                snprintf(statusCodeString, 60, "Access Denied");
                break;
            case NVME_MED_ERR_SC_DEALLOCATED_OR_UNWRITTEN_LOGICAL_BLOCK:
                snprintf(statusCodeString, 60, "Deallocated or Unwritten Logical Block");
                break;
            default:
                snprintf(statusCodeString, 60, "Unknown");
                break;
            }
            break;
        case NVME_SCT_VENDOR_SPECIFIC_STATUS:
            snprintf(statusCodeTypeString, 60, "Vendor Specific");
            snprintf(statusCodeString, 60, "Unknown");
            break;
        default:
            snprintf(statusCodeTypeString, 60, "Unknown");
            snprintf(statusCodeString, 60, "Unknown");
            break;
        }
        printf("\t\tStatus Code Type: %s (%" PRIX8 "h)\n", statusCodeTypeString, statusCodeType);
        printf("\t\tStatus Code: %s (%" PRIX8 "h)\n", statusCodeString, statusCode);
    }
    else
    {
        printf("Unavailable from OS\n");
    }
    printf("\n");
}

char *nvme_cmd_to_string(int admin, uint8_t opcode)
{
    if (admin) {
        switch (opcode) {
        case NVME_ADMIN_CMD_DELETE_SQ:  return "Delete I/O Submission Queue";
        case NVME_ADMIN_CMD_CREATE_SQ:  return "Create I/O Submission Queue";
        case NVME_ADMIN_CMD_GET_LOG_PAGE:   return "Get Log Page";
        case NVME_ADMIN_CMD_DELETE_CQ:  return "Delete I/O Completion Queue";
        case NVME_ADMIN_CMD_CREATE_CQ:  return "Create I/O Completion Queue";
        case NVME_ADMIN_CMD_IDENTIFY:   return "Identify";
        case NVME_ADMIN_CMD_ABORT_CMD:  return "Abort";
        case NVME_ADMIN_CMD_SET_FEATURES:   return "Set Features";
        case NVME_ADMIN_CMD_GET_FEATURES:   return "Get Features";
        case NVME_ADMIN_CMD_ASYNC_EVENT:    return "Asynchronous Event Request";
        case NVME_ADMIN_CMD_NAMESPACE_MANAGEMENT:   return "Namespace Management";
        case NVME_ADMIN_CMD_ACTIVATE_FW:    return "Firmware Commit";
        case NVME_ADMIN_CMD_DOWNLOAD_FW:    return "Firmware Image Download";
        case NVME_ADMIN_CMD_DEVICE_SELF_TEST:   return "Device Self-test";
        case NVME_ADMIN_CMD_NAMESPACE_ATTACHMENT:   return "Namespace Attachment";
        case NVME_ADMIN_CMD_KEEP_ALIVE: return "Keep Alive";
        case NVME_ADMIN_CMD_DIRECTIVE_SEND: return "Directive Send";
        case NVME_ADMIN_CMD_DIRECTIVE_RECEIVE:  return "Directive Receive";
        case NVME_ADMIN_CMD_VIRTUALIZATION_MANAGEMENT:  return "Virtualization Management";
        case NVME_ADMIN_CMD_NVME_MI_SEND:   return "NVMEe-MI Send";
        case NVME_ADMIN_CMD_NVME_MI_RECEIVE:    return "NVMEe-MI Receive";
        case NVME_ADMIN_CMD_DOORBELL_BUFFER_CONFIG:     return "Doorbell Buffer Config";
        case NVME_ADMIN_CMD_NVME_OVER_FABRICS:      return "NVMe Over Fabric";
        case NVME_ADMIN_CMD_FORMAT_NVM: return "Format NVM";
        case NVME_ADMIN_CMD_SECURITY_SEND:  return "Security Send";
        case NVME_ADMIN_CMD_SECURITY_RECV:  return "Security Receive";
        case NVME_ADMIN_CMD_SANITIZE:   return "Sanitize";
        }
    } else {
        switch (opcode) {
        case NVME_CMD_FLUSH:        return "Flush";
        case NVME_CMD_WRITE:        return "Write";
        case NVME_CMD_READ:     return "Read";
        case NVME_CMD_WRITE_UNCOR:  return "Write Uncorrectable";
        case NVME_CMD_COMPARE:      return "Compare";
        case NVME_CMD_WRITE_ZEROS:  return "Write Zeroes";
        case NVME_CMD_DATA_SET_MANAGEMENT:      return "Dataset Management";
        case NVME_CMD_RESERVATION_REGISTER: return "Reservation Register";
        case NVME_CMD_RESERVATION_REPORT:   return "Reservation Report";
        case NVME_CMD_RESERVATION_ACQUIRE:  return "Reservation Acquire";
        case NVME_CMD_RESERVATION_RELEASE:  return "Reservation Release";
        }
    }

    return "Unknown";
}

int nvme_Get_SMART_Log_Page(tDevice *device, uint32_t nsid, uint8_t * pData, uint32_t dataLen)
{
    int ret = UNKNOWN;
    nvmeGetLogPageCmdOpts cmdOpts;
    nvmeSmartLog * smartLog; // in case we need to align memory
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif
    if ( (pData == NULL) || (dataLen < NVME_SMART_HEALTH_LOG_LEN) )
    {
        return ret;
    }

    memset(&cmdOpts,0,sizeof(nvmeGetLogPageCmdOpts));
    smartLog = C_CAST(nvmeSmartLog *, pData);

    cmdOpts.nsid = nsid;
    //cmdOpts.addr = C_CAST(uint64_t, smartLog);
    cmdOpts.addr = C_CAST(uint8_t*, smartLog);
    cmdOpts.dataLen = NVME_SMART_HEALTH_LOG_LEN;
    cmdOpts.lid = NVME_LOG_SMART_ID;

    ret = nvme_Get_Log_Page(device, &cmdOpts);
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Get_ERROR_Log_Page(tDevice *device, uint8_t * pData, uint32_t dataLen)
{
    int ret = UNKNOWN;
    nvmeGetLogPageCmdOpts cmdOpts;
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif
    //Should be able to pull at least one entry. 
    if ( (pData == NULL) || (dataLen < sizeof(nvmeErrLogEntry)) )
    {
        return ret;
    }
   
    memset(&cmdOpts,0,sizeof(nvmeGetLogPageCmdOpts));
    cmdOpts.addr = pData;
    cmdOpts.dataLen = dataLen;
    cmdOpts.lid = NVME_LOG_ERROR_ID;

    ret = nvme_Get_Log_Page(device, &cmdOpts);
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Get_FWSLOTS_Log_Page(tDevice *device, uint8_t * pData, uint32_t dataLen)
{
    int ret = UNKNOWN;
    nvmeGetLogPageCmdOpts cmdOpts;
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif
    //Should be able to pull at least one entry. 
    if ( (pData == NULL) || (dataLen < sizeof(nvmeFirmwareSlotInfo)) )
    {
        return ret;
    }
   
    memset(&cmdOpts,0,sizeof(nvmeGetLogPageCmdOpts));
    cmdOpts.addr = pData;
    cmdOpts.dataLen = dataLen;
    cmdOpts.lid = NVME_LOG_FW_SLOT_ID;
    
    ret = nvme_Get_Log_Page(device, &cmdOpts);
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Get_CmdSptEfft_Log_Page(tDevice *device, uint8_t * pData, uint32_t dataLen)
{
    int ret = UNKNOWN;
    nvmeGetLogPageCmdOpts cmdOpts;
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif
    //Should be able to pull at least one entry. 
    if ( (pData == NULL) || (dataLen < sizeof(nvmeFirmwareSlotInfo)) )
    {
        return ret;
    }
   
    memset(&cmdOpts,0,sizeof(nvmeGetLogPageCmdOpts));
    cmdOpts.addr = pData;
    cmdOpts.dataLen = dataLen;
    cmdOpts.lid = NVME_LOG_CMD_SPT_EFET_ID;
    
    ret = nvme_Get_Log_Page(device, &cmdOpts);
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Get_DevSelfTest_Log_Page(tDevice *device, uint8_t * pData, uint32_t dataLen)
{
    int ret = UNKNOWN;
    nvmeGetLogPageCmdOpts cmdOpts;
#ifdef _DEBUG
    printf("-->%s\n",__FUNCTION__);
#endif
    //Should be able to pull at least one entry. 
    if ( (pData == NULL) || (dataLen < sizeof(nvmeFirmwareSlotInfo)) )
    {
        return ret;
    }
   
    memset(&cmdOpts,0,sizeof(nvmeGetLogPageCmdOpts));
    cmdOpts.addr = pData;
    cmdOpts.dataLen = dataLen;
    cmdOpts.lid = NVME_LOG_DEV_SELF_TEST;
    
    ret = nvme_Get_Log_Page(device, &cmdOpts);
#ifdef _DEBUG
    printf("<--%s (%d)\n",__FUNCTION__, ret);
#endif
    return ret;
}
//Seagate unique?
int nvme_Read_Ext_Smt_Log(tDevice *device, EXTENDED_SMART_INFO_T *ExtdSMARTInfo)
{
    int ret = SUCCESS;
    nvmeGetLogPageCmdOpts getExtSMARTLog;
    memset(&getExtSMARTLog, 0, sizeof(nvmeGetLogPageCmdOpts));
    getExtSMARTLog.dataLen = sizeof(EXTENDED_SMART_INFO_T);
    getExtSMARTLog.lid = 0xC4;
    getExtSMARTLog.nsid = device->drive_info.namespaceID;
    getExtSMARTLog.addr = C_CAST(uint8_t*, ExtdSMARTInfo);

    if (VERBOSITY_COMMAND_NAMES <= device->deviceVerbosity)
    {
        printf("Reading NVMe Ext SMART Log\n");
    }
    ret = nvme_Get_Log_Page(device, &getExtSMARTLog);
    if (VERBOSITY_COMMAND_NAMES <= device->deviceVerbosity)
    {
        print_Return_Enum("Read Ext SMART Log", ret);
    }
    return ret;
}

#endif
