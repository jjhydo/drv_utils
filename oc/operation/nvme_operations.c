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

#if !defined (DISABLE_NVME_PASSTHROUGH)
#include "nvme_operations.h"

void nvme_Print_Feature_Identifiers_Help(void)
{
    printf("\n====================================================\n");
    printf(" Feature\t O/M \tPersistent\tDescription\n");
    printf("Identifier\t   \tAcross Power\t      \n");
    printf("          \t   \t  & Reset   \t      \n");
    printf("====================================================\n");
    printf("00h       \t   \t            \tReserved\n");
    printf("01h       \t M \t   NO       \tArbitration\n");
    printf("02h       \t M \t   NO       \tPower Management\n");
    printf("03h       \t O \t   YES      \tLBA Range Type\n");
    printf("04h       \t M \t   NO       \tTemprature Threshold\n");
    printf("05h       \t M \t   NO       \tError Recovery\n");
    printf("06h       \t O \t   NO       \tVolatile Write Cache\n");
    printf("07h       \t M \t   NO       \tNumber of Queues\n");
    printf("08h       \t M \t   NO       \tInterrupt Coalescing\n");
    printf("09h       \t M \t   NO       \tInterrupt Vector Configuration\n");
    printf("0Ah       \t M \t   NO       \tWrite Atomicity Normal\n");
    printf("0Bh       \t M \t   NO       \tAsynchronous Event Configuration\n");
    printf("0Ch       \t O \t   NO       \tAutonomous Power State Transition\n");
    printf("0Dh       \t O \t   NO       \tHost Memory Buffer\n");
    printf("0Eh-77h   \t   \t            \tReserved          \n");
    printf("78h-7Fh   \t   \t            \tRefer to NVMe Management Spec\n");
    printf("80h-BFh   \t   \t            \tCommand Set Specific (Reserved)\n");
    printf("C0h-FFh   \t   \t            \tVendor Specific\n");
    printf("====================================================\n");
}

int nvme_Print_All_Feature_Identifiers(tDevice *device, eNvmeFeaturesSelectValue selectType, M_ATTR_UNUSED bool listOnlySupportedFeatures)
{
    int ret = UNKNOWN;
    uint8_t featureID;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    printf(" Feature ID\tRaw Value\n");
    printf("===============================\n");
    for (featureID = 1; featureID < 0x0C; featureID++)
    {
        if (featureID == NVME_FEAT_LBA_RANGE_)
        {
            continue;
        }
        memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
        featureCmd.fid = featureID;
        featureCmd.sel = selectType;
        if (nvme_Get_Features(device, &featureCmd) == SUCCESS)
        {
            printf("    %02Xh    \t0x%08X\n", featureID, featureCmd.featSetGetValue);
        }
    }
    printf("===============================\n");

#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Print_Arbitration_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_ARBITRATION_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tArbitration & Command Processing Feature\n");
        printf("=============================================\n");
        printf("Hi  Priority Weight (HPW) :\t\t0x%02X\n", ((featureCmd.featSetGetValue & 0xFF000000) >> 24));
        printf("Med Priority Weight (MPW) :\t\t0x%02X\n", ((featureCmd.featSetGetValue & 0x00FF0000) >> 16));
        printf("Low Priority Weight (LPW) :\t\t0x%02X\n", ((featureCmd.featSetGetValue & 0x0000FF00) >> 8));
        printf("Arbitration Burst    (AB) :\t\t0x%02X\n", featureCmd.featSetGetValue & 0x00000003);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}


//Temperature Threshold 
int nvme_Print_Temperature_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
    uint8_t   TMPSEL = 0; //0=Composite, 1=Sensor 1, 2=Sensor 2, ...
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_TEMP_THRESH_;
    featureCmd.sel = selectType;
    printf("\n\tTemperature Threshold Feature\n");
    printf("=============================================\n");
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("Composite Temperature : \t0x%04X\tOver  Temp. Threshold\n", \
            (featureCmd.featSetGetValue & 0x000000FF));
    }
    featureCmd.featSetGetValue = BIT20;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("Composite Temperature : \t0x%04X\tUnder Temp. Threshold\n", \
            (featureCmd.featSetGetValue & 0x000000FF));
    }

    for (TMPSEL = 1; TMPSEL <= 8; TMPSEL++)
    {
        featureCmd.featSetGetValue = (TMPSEL << 16);
        ret = nvme_Get_Features(device, &featureCmd);
        if (ret == SUCCESS)
        {
            printf("Temperature Sensor %d  : \t0x%04X\tOver  Temp. Threshold\n", \
                TMPSEL, (featureCmd.featSetGetValue & 0x000000FF));
        }
        //Not get Under Temprature 
        // BIT20 = THSEL 0=Over Temperature Thresh. 1=Under Temperature Thresh. 
        featureCmd.featSetGetValue = C_CAST(uint32_t, (BIT20 | (C_CAST(uint32_t, TMPSEL) << 16)));
        ret = nvme_Get_Features(device, &featureCmd);
        if (ret == SUCCESS)
        {
            printf("Temperature Sensor %d  : \t0x%04X\tUnder Temp. Threshold\n", \
                TMPSEL, (featureCmd.featSetGetValue & 0x000000FF));
        }
    }
    //WARNING: This is just sending back the last sensor ret 
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Power Management
int nvme_Print_PM_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_POWER_MGMT_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tPower Management Feature Details\n");
        printf("=============================================\n");
        printf("Workload Hint  (WH) :\t\t0x%02X\n", ((featureCmd.featSetGetValue & 0x000000E0) >> 5));
        printf("Power State    (PS) :\t\t0x%02X\n", featureCmd.featSetGetValue & 0x0000001F);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Error Recovery
int nvme_Print_Error_Recovery_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_ERR_RECOVERY_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tError Recovery Feature Details\n");
        printf("=============================================\n");
        printf("Deallocated Logical Block Error (DULBE) :\t\t%s\n", (featureCmd.featSetGetValue & BIT16) ? "Enabled" : "Disabled");
        printf("Time Limited Error Recovery     (TLER)  :\t\t0x%04X\n", featureCmd.featSetGetValue & 0x0000FFFF);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Volatile Write Cache Feature. 
int nvme_Print_WCE_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_VOLATILE_WC_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tVolatile Write Cache Feature Details\n");
        printf("=============================================\n");
        printf("Volatile Write Cache (WCE) :\t\t%s\n", (featureCmd.featSetGetValue & BIT0) ? "Enabled" : "Disabled");

    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Number of Queues Feature 
int nvme_Print_NumberOfQueues_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_NUM_QUEUES_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tNumber of Queues Feature Details\n");
        printf("=============================================\n");
        printf("# of I/O Completion Queues Requested (NCQR)  :\t\t0x%04X\n", (featureCmd.featSetGetValue & 0xFFFF0000) >> 16);
        printf("# of I/O Submission Queues Requested (NSQR)  :\t\t0x%04X\n", featureCmd.featSetGetValue & 0x0000FFFF);

        //TODO: How to get NCQA??? Seems like Linux driver limitation? -X
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Interrupt Coalescing (08h Feature)
int nvme_Print_Intr_Coalescing_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_IRQ_COALESCE_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tInterrupt Coalescing Feature Details\n");
        printf("=============================================\n");
        printf("Aggregation Time     (TIME)  :\t\t0x%02X\n", (featureCmd.featSetGetValue & 0x0000FF00) >> 8);
        printf("Aggregation Threshold (THR)  :\t\t0x%02X\n", featureCmd.featSetGetValue & 0x000000FF);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Interrupt Vector Configuration (09h Feature)
int nvme_Print_Intr_Config_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_IRQ_CONFIG_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tInterrupt Vector Configuration Feature Details\n");
        printf("=============================================\n");
        printf("Coalescing Disable (CD) :\t%s\n", (featureCmd.featSetGetValue & BIT16) ? "Enabled" : "Disabled");
        printf("Interrupt Vector   (IV) :\t0x%02X\n", featureCmd.featSetGetValue & 0x000000FF);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Write Atomicity Normal (0Ah Feature)
int nvme_Print_Write_Atomicity_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_WRITE_ATOMIC_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tWrite Atomicity Normal Feature Details\n");
        printf("=============================================\n");
        printf("Disable Normal (DN) :\t%s\n\n", (featureCmd.featSetGetValue & BIT0) ? "Enabled" : "Disabled");
        if (featureCmd.featSetGetValue & BIT0)
        {
            printf(" Host specifies that AWUN & NAWUN are not required\n");
            printf(" & controller shall only honor AWUPF & NAWUPF\n");
        }
        else
        {
            printf("Controller honors AWUN, NAWUN, AWUPF & NAWUPF\n");
        }
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

//Asynchronous Event Configuration (0Bh Feature)
int nvme_Print_Async_Config_Feature_Details(tDevice *device, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
    nvmeFeaturesCmdOpt featureCmd;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&featureCmd, 0, sizeof(nvmeFeaturesCmdOpt));
    featureCmd.fid = NVME_FEAT_ASYNC_EVENT_;
    featureCmd.sel = selectType;
    ret = nvme_Get_Features(device, &featureCmd);
    if (ret == SUCCESS)
    {
        printf("\n\tAsync Event Configuration\n");
        printf("=============================================\n");
        printf("Firmware Activation Notices     :\t%s\n", (featureCmd.featSetGetValue & BIT9) ? "Enabled" : "Disabled");
        printf("Namespace Attribute Notices     :\t%s\n", (featureCmd.featSetGetValue & BIT8) ? "Enabled" : "Disabled");
        printf("SMART/Health Critical Warnings  :\t0x%02X\n", featureCmd.featSetGetValue & 0x000000FF);
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Print_Feature_Details(tDevice *device, uint8_t featureID, eNvmeFeaturesSelectValue selectType)
{
    int ret = UNKNOWN;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    switch (featureID)
    {
    case NVME_FEAT_ARBITRATION_:
        ret = nvme_Print_Arbitration_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_POWER_MGMT_:
        ret = nvme_Print_PM_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_TEMP_THRESH_:
        ret = nvme_Print_Temperature_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_ERR_RECOVERY_:
        ret = nvme_Print_Error_Recovery_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_VOLATILE_WC_:
        ret = nvme_Print_WCE_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_NUM_QUEUES_:
        ret = nvme_Print_NumberOfQueues_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_IRQ_COALESCE_:
        ret = nvme_Print_Intr_Coalescing_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_IRQ_CONFIG_:
        ret = nvme_Print_Intr_Config_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_WRITE_ATOMIC_:
        ret = nvme_Print_Write_Atomicity_Feature_Details(device, selectType);
        break;
    case NVME_FEAT_ASYNC_EVENT_:
        ret = nvme_Print_Async_Config_Feature_Details(device, selectType);
        break;
    default:
        ret = NOT_SUPPORTED;
        break;
    }
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Get_Log_Size(uint8_t logPageId, uint64_t * logSize)
{
    switch (logPageId)
    {
    case NVME_LOG_ERROR_ID:
        *logSize = sizeof(nvmeErrLogEntry);
        break;
    case NVME_LOG_SMART_ID:
    case NVME_LOG_FW_SLOT_ID: //Same size as Health
        *logSize = NVME_SMART_HEALTH_LOG_LEN;
        break;
    case NVME_LOG_CMD_SPT_EFET_ID:
        *logSize = sizeof(nvmeEffectsLog);
        break;
    case NVME_LOG_DEV_SELF_TEST:
        *logSize = sizeof(nvmeSelfTestLog);
        break;
    default:
        *logSize = 0;
        break;
    }
    return SUCCESS; // Can be used later to tell if the log is unavailable or we can't get size. 
}

int nvme_Print_FWSLOTS_Log_Page(tDevice *device)
{
    int ret = UNKNOWN;
    int slot = 0;
    nvmeFirmwareSlotInfo fwSlotsLogInfo;
    char fwRev[9]; // 8 bytes for the FSR + 1 byte '\0'
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    memset(&fwSlotsLogInfo, 0, sizeof(nvmeFirmwareSlotInfo));
    ret = nvme_Get_FWSLOTS_Log_Page(device, (uint8_t*)&fwSlotsLogInfo, sizeof(nvmeFirmwareSlotInfo));
    if (ret == SUCCESS)
    {
#ifdef _DEBUG
        printf("AFI: 0x%X\n", fwSlotsLogInfo.afi);
#endif
        printf("\nFirmware slot actively running firmware: %d\n", fwSlotsLogInfo.afi & 0x03);
        printf("Firmware slot to be activated at next reset: %d\n\n", ((fwSlotsLogInfo.afi & 0x30) >> 4));

        for (slot = 1; slot <= NVME_MAX_FW_SLOTS; slot++)
        {
            memcpy(fwRev, (char *)&fwSlotsLogInfo.FSR[slot - 1], 8);
            fwRev[8] = '\0';
            printf(" Slot %d : %s\n", slot, fwRev);
        }
    }

#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

void show_effects_log_human(uint32_t effect)
{
    const char *set = "+";
    const char *clr = "-";

    printf("  CSUPP+");
    printf("  LBCC%s", (effect & NVME_CMD_EFFECTS_LBCC) ? set : clr);
    printf("  NCC%s", (effect & NVME_CMD_EFFECTS_NCC) ? set : clr);
    printf("  NIC%s", (effect & NVME_CMD_EFFECTS_NIC) ? set : clr);
    printf("  CCC%s", (effect & NVME_CMD_EFFECTS_CCC) ? set : clr);

    if ((effect & NVME_CMD_EFFECTS_CSE_MASK) >> 16 == 0)
        printf("  No command restriction\n");
    else if ((effect & NVME_CMD_EFFECTS_CSE_MASK) >> 16 == 1)
        printf("  No other command for same namespace\n");
    else if ((effect & NVME_CMD_EFFECTS_CSE_MASK) >> 16 == 2)
        printf("  No other command for any namespace\n");
    else
        printf("  Reserved CSE\n");
}

int nvme_Print_CmdSptEfft_Log_Page(tDevice *device)
{
    int ret = UNKNOWN;
    nvmeEffectsLog effectsLogInfo;
    uint16_t i = 0;
    uint32_t effect = 0;

#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif

    memset(&effectsLogInfo, 0, sizeof(nvmeEffectsLog));
    ret = nvme_Get_CmdSptEfft_Log_Page(device, (uint8_t*)&effectsLogInfo, sizeof(nvmeEffectsLog));
    if (ret == SUCCESS)
    {
        printf("Admin Command Set\n");
        for (i = 0; i < 256; i++)
        {
            effect = effectsLogInfo.acs[i];
            if (effect & NVME_CMD_EFFECTS_CSUPP)
            {
                printf("ACS%-6" PRIu16 "[%-32s] %08" PRIX32, i, nvme_cmd_to_string(1, C_CAST(uint8_t, i)), effect);
                show_effects_log_human(effect);
            }
        }
        printf("\nNVM Command Set\n");
        for (i = 0; i < 256; i++)
        {
            effect = effectsLogInfo.iocs[i];
            if (effect & NVME_CMD_EFFECTS_CSUPP)
            {
                printf("IOCS%-5" PRIu16 "[%-32s] %08" PRIX32, i, nvme_cmd_to_string(0, C_CAST(uint8_t, i)), effect);
                show_effects_log_human(effect);
            }
        }
    }

#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}


int nvme_Print_DevSelfTest_Log_Page(tDevice *device)
{
    int ret = UNKNOWN;
    nvmeSelfTestLog selfTestLogInfo;
    int i, temp;
    const char *test_code_res;
    const char *test_res[10] = {
        "Operation completed without error",
        "Operation was aborted by a Device Self-test command",
        "Operation was aborted by a Controller Level Reset",
        "Operation was aborted due to a removal of a namespace from the namespace inventory",
        "Operation was aborted due to the processing of a Format NVM command",
        "A fatal error or unknown test error occurred while the controller was executing the"\
        " device self-test operation andthe operation did not complete",
        "Operation completed with a segment that failed and the segment that failed is not known",
        "Operation completed with one or more failed segments and the first segment that failed "\
        "is indicated in the SegmentNumber field",
        "Operation was aborted for unknown reason",
        "Reserved"
    };


#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif

    memset(&selfTestLogInfo, 0, sizeof(nvmeSelfTestLog));
    ret = nvme_Get_DevSelfTest_Log_Page(device, (uint8_t*)&selfTestLogInfo, sizeof(nvmeSelfTestLog));
    if (ret == SUCCESS)
    {
        printf("Current operation : %#x\n", selfTestLogInfo.crntDevSelftestOprn);
        printf("Current Completion : %u%%\n", selfTestLogInfo.crntDevSelftestCompln);
        for (i = 0; i < NVME_SELF_TEST_REPORTS; i++)
        {
            temp = selfTestLogInfo.result[i].deviceSelfTestStatus & 0xf;
            if (temp == 0xf)
                continue;

            printf("Result[%d]:\n", i);
            printf("  Test Result                  : %#x %s\n", temp,
                test_res[temp > 9 ? 9 : temp]);

            temp = selfTestLogInfo.result[i].deviceSelfTestStatus >> 4;
            switch (temp) {
            case 1:
                test_code_res = "Short device self-test operation";
                break;
            case 2:
                test_code_res = "Extended device self-test operation";
                break;
            case 0xe:
                test_code_res = "Vendor specific";
                break;
            default:
                test_code_res = "Reserved";
                break;
            }
            printf("  Test Code                    : %#x %s\n", temp,
                test_code_res);
            if (temp == 7)
                printf("  Segment number               : %#x\n",
                    selfTestLogInfo.result[i].segmentNum);

            temp = selfTestLogInfo.result[i].validDiagnosticInfo;
            printf("  Valid Diagnostic Information : %#x\n", temp);
            printf("  Power on hours (POH)         : %#"PRIx64"\n", selfTestLogInfo.result[i].powerOnHours);

            if (temp & NVME_SELF_TEST_VALID_NSID)
                printf("  Namespace Identifier         : %#x\n", selfTestLogInfo.result[i].nsid);

            if (temp & NVME_SELF_TEST_VALID_FLBA)
                printf("  Failing LBA                  : %#"PRIx64"\n", selfTestLogInfo.result[i].failingLba);

            if (temp & NVME_SELF_TEST_VALID_SCT)
                printf("  Status Code Type             : %#x\n", selfTestLogInfo.result[i].statusCodeType);

            if (temp & NVME_SELF_TEST_VALID_SC)
                printf("  Status Code                  : %#x\n", selfTestLogInfo.result[i].statusCode);

            printf("  Vendor Specific                      : %x %x\n", selfTestLogInfo.result[i].vendorSpecific[0], selfTestLogInfo.result[i].vendorSpecific[1]);
        }
    }

#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

int nvme_Print_ERROR_Log_Page(tDevice *device, uint64_t numOfErrToPrint)
{
    int ret = UNKNOWN;
    int err = 0;
    nvmeErrLogEntry * pErrLogBuf = NULL;
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    //TODO: If this is not specified get the value. 
    if (!numOfErrToPrint)
    {
        numOfErrToPrint = 32;
    }
    pErrLogBuf = C_CAST(nvmeErrLogEntry *, calloc_aligned(C_CAST(size_t, numOfErrToPrint), sizeof(nvmeErrLogEntry), device->os_info.minimumAlignment));
    if (pErrLogBuf != NULL)
    {
        ret = nvme_Get_ERROR_Log_Page(device, C_CAST(uint8_t*, pErrLogBuf), C_CAST(uint32_t, numOfErrToPrint * sizeof(nvmeErrLogEntry)));
        if (ret == SUCCESS)
        {
            printf("Err #\tLBA\t\tSQ ID\tCMD ID\tStatus\tLocation\n");
            printf("=======================================================\n");
            for (err = 0; err < C_CAST(int, numOfErrToPrint); err++)
            {
                if (pErrLogBuf[err].errorCount)
                {

                    printf("%"PRIu64"\t%"PRIu64"\t%"PRIu16"\t%"PRIu16"\t0x%02"PRIX16"\t0x%02"PRIX16"\n", \
                        pErrLogBuf[err].errorCount,
                        pErrLogBuf[err].lba,
                        pErrLogBuf[err].subQueueID,
                        pErrLogBuf[err].cmdID,
                        pErrLogBuf[err].statusField,
                        pErrLogBuf[err].paramErrLocation);
                }
            }
        }
    }
    safe_Free_aligned(pErrLogBuf)
#ifdef _DEBUG
    printf("<--%s (%d)\n", __FUNCTION__, ret);
#endif
    return ret;
}

int print_Nvme_Ctrl_Regs(tDevice * device)
{
    int ret = UNKNOWN;

    nvmeBarCtrlRegisters ctrlRegs;

    memset(&ctrlRegs, 0, sizeof(nvmeBarCtrlRegisters));

    printf("\n=====CONTROLLER REGISTERS=====\n");

    ret = nvme_Read_Ctrl_Reg(device, &ctrlRegs);

    if (ret == SUCCESS)
    {
        printf("Controller Capabilities (CAP)\t:\t%"PRIx64"\n", ctrlRegs.cap);
        printf("Version (VS)\t:\t0x%x\n", ctrlRegs.vs);
        printf("Interrupt Mask Set (INTMS)\t:\t0x%x\n", ctrlRegs.intms);
        printf("Interrupt Mask Clear (INTMC)\t:\t0x%x\n", ctrlRegs.intmc);
        printf("Controller Configuration (CC)\t:\t0x%x\n", ctrlRegs.cc);
        printf("Controller Status (CSTS)\t:\t0x%x\n", ctrlRegs.csts);
        //What about NSSR?
        printf("Admin Queue Attributes (AQA)\t:\t0x%x\n", ctrlRegs.aqa);
        printf("Admin Submission Queue Base Address (ASQ)\t:\t%"PRIx64"\n", ctrlRegs.asq);
        printf("Admin Completion Queue Base Address (ACQ)\t:\t%"PRIx64"\n", ctrlRegs.acq);
    }
    else
    {
        printf("Couldn't read Controller register for dev %s\n", device->os_info.name);
    }
    return ret;
}

/***************************************
* Extended-SMART Information
***************************************/
char* print_ext_smart_id(uint8_t attrId)
{
    switch (attrId) {
    case VS_ATTR_ID_SOFT_READ_ERROR_RATE:
        return "Soft ECC error count";
    case VS_ATTR_ID_REALLOCATED_SECTOR_COUNT:
        return "Bad NAND block count";
    case VS_ATTR_ID_POWER_ON_HOURS:
        return "Power On Hours";
    case VS_ATTR_ID_POWER_FAIL_EVENT_COUNT:
        return "Power Fail Event Count";
    case VS_ATTR_ID_DEVICE_POWER_CYCLE_COUNT:
        return "Device Power Cycle Count";
    case VS_ATTR_ID_RAW_READ_ERROR_RATE:
        return "Uncorrectable read error count";
        /**********************************************
                case 30:
                    return "LIFETIME_WRITES0_TO_FLASH";
                case 31:
                    return "LIFETIME_WRITES1_TO_FLASH";
                case 32:
                    return "LIFETIME_WRITES0_FROM_HOST";
                case 33:
                    return "LIFETIME_WRITES1_FROM_HOST";
                case 34:
                    return "LIFETIME_READ0_FROM_HOST";
                case 35:
                    return "LIFETIME_READ1_FROM_HOST";
                case 36:
                    return "PCIE_PHY_CRC_ERROR";
                case 37:
                    return "BAD_BLOCK_COUNT_SYSTEM";
                case 38:
                    return "BAD_BLOCK_COUNT_USER";
                case 39:
                    return "THERMAL_THROTTLING_STATUS";
        **********************************************/
    case VS_ATTR_ID_GROWN_BAD_BLOCK_COUNT:
        return "Bad NAND block count";
    case VS_ATTR_ID_END_2_END_CORRECTION_COUNT:
        return "SSD End to end correction counts";
    case VS_ATTR_ID_MIN_MAX_WEAR_RANGE_COUNT:
        return "User data erase counts";
    case VS_ATTR_ID_REFRESH_COUNT:
        return "Refresh count";
    case VS_ATTR_ID_BAD_BLOCK_COUNT_USER:
        return "User data erase fail count";
    case VS_ATTR_ID_BAD_BLOCK_COUNT_SYSTEM:
        return "System area erase fail count";
    case VS_ATTR_ID_THERMAL_THROTTLING_STATUS:
        return "Thermal throttling status and count";
    case VS_ATTR_ID_ALL_PCIE_CORRECTABLE_ERROR_COUNT:
        return "PCIe Correctable Error count";
    case VS_ATTR_ID_ALL_PCIE_UNCORRECTABLE_ERROR_COUNT:
        return "PCIe Uncorrectable Error count";
    case VS_ATTR_ID_INCOMPLETE_SHUTDOWN_COUNT:
        return "Incomplete shutdowns";
    case VS_ATTR_ID_GB_ERASED_LSB:
        return "LSB of Flash GB erased";
    case VS_ATTR_ID_GB_ERASED_MSB:
        return "MSB of Flash GB erased";
    case VS_ATTR_ID_LIFETIME_DEVSLEEP_EXIT_COUNT:
        return "LIFETIME_DEV_SLEEP_EXIT_COUNT";
    case VS_ATTR_ID_LIFETIME_ENTERING_PS4_COUNT:
        return "LIFETIME_ENTERING_PS4_COUNT";
    case VS_ATTR_ID_LIFETIME_ENTERING_PS3_COUNT:
        return "LIFETIME_ENTERING_PS3_COUNT";
    case VS_ATTR_ID_RETIRED_BLOCK_COUNT:
        return "Retired block count"; /*VS_ATTR_ID_RETIRED_BLOCK_COUNT*/
    case VS_ATTR_ID_PROGRAM_FAILURE_COUNT:
        return "Program fail count";
    case VS_ATTR_ID_ERASE_FAIL_COUNT:
        return "Erase Fail Count";
    case VS_ATTR_ID_AVG_ERASE_COUNT:
        return "System data % used";
    case VS_ATTR_ID_UNEXPECTED_POWER_LOSS_COUNT:
        return "Unexpected power loss count";
    case VS_ATTR_ID_WEAR_RANGE_DELTA:
        return "Wear range delta";
    case VS_ATTR_ID_SATA_INTERFACE_DOWNSHIFT_COUNT:
        return "PCIE_INTF_DOWNSHIFT_COUNT";
    case VS_ATTR_ID_END_TO_END_CRC_ERROR_COUNT:
        return "E2E_CRC_ERROR_COUNT";
    case VS_ATTR_ID_UNCORRECTABLE_ECC_ERRORS:
        return "Soft ECC error count";
    case VS_ATTR_ID_MAX_LIFE_TEMPERATURE:
        return "Max lifetime temperature";/*VS_ATTR_ID_MAX_LIFE_TEMPERATURE for extended*/
    case VS_ATTR_ID_RAISE_ECC_CORRECTABLE_ERROR_COUNT:
        return "RAIS_ECC_CORRECT_ERR_COUNT";
    case VS_ATTR_ID_UNCORRECTABLE_RAISE_ERRORS:
        return "Uncorrectable read error count";/*VS_ATTR_ID_UNCORRECTABLE_RAISE_ERRORS*/
    case VS_ATTR_ID_DRIVE_LIFE_PROTECTION_STATUS:
        return "DRIVE_LIFE_PROTECTION_STATUS";
    case VS_ATTR_ID_REMAINING_SSD_LIFE:
        return "Remaining SSD life";
    case VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_LSB:
        return "LSB of Physical (NAND) bytes written";
    case VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_MSB:
        return "MSB of Physical (NAND) bytes written";
    case VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_LSB:
        return "LSB of Physical (HOST) bytes written";
    case VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_MSB:
        return "MSB of Physical (HOST) bytes written";
    case VS_ATTR_ID_LIFETIME_READS_TO_HOST_LSB:
        return "LSB of Physical (NAND) bytes read";
    case VS_ATTR_ID_LIFETIME_READS_TO_HOST_MSB:
        return "MSB of Physical (NAND) bytes read";
    case VS_ATTR_ID_FREE_SPACE:
        return "Free Space";
    case VS_ATTR_ID_TRIM_COUNT_LSB:
        return "LSB of Trim count";
    case VS_ATTR_ID_TRIM_COUNT_MSB:
        return "MSB of Trim count";
    case VS_ATTR_ID_OP_PERCENTAGE:
        return "OP percentage";
    case VS_ATTR_ID_MAX_SOC_LIFE_TEMPERATURE:
        return "Max lifetime SOC temperature";
    default:
        return "Un-Known";
    }
}

uint64_t smart_attribute_vs(uint16_t verNo, SmartVendorSpecific attr)
{
    uint64_t val = 0;
    fb_smart_attribute_data *attrFb;

    /**
     * These are all FaceBook specific attributes.
     */
    if (verNo >= EXTENDED_SMART_VERSION_FB) {
        attrFb = (fb_smart_attribute_data *)&attr;
        val = attrFb->MSDword;
        val = (val << 32) | attrFb->LSDword;
        return val;
    }

    /******************************************************************
        if(attr.AttributeNumber == VS_ATTR_POWER_CONSUMPTION) {
            attrFb = (fb_smart_attribute_data *)&attr;
            return attrFb->LSDword;
        }
        else if(attr.AttributeNumber == VS_ATTR_THERMAL_THROTTLING_STATUS) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            return attrFb->LSDword;
        }
        else if(attr.AttributeNumber == VS_ATTR_PCIE_PHY_CRC_ERROR) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            return attrFb->LSDword;
        }
        else if(attr.AttributeNumber == VS_ATTR_BAD_BLOCK_COUNT_USER) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            return attrFb->LSDword;
        }
        else if(attr.AttributeNumber == VS_ATTR_BAD_BLOCK_COUNT_SYSTEM) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            return attrFb->LSDword;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_READ1_FROM_HOST) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_READ0_FROM_HOST) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_WRITES1_FROM_HOST) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_WRITES0_FROM_HOST) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_WRITES1_TO_FLASH) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
        else if(attr.AttributeNumber == VS_ATTR_LIFETIME_WRITES0_TO_FLASH) {
            fb_smart_attribute_data *attrFb;
            attrFb = (fb_smart_attribute_data *)&attr;
            val = attrFb->MSDword;
            val = (val << 32) | attrFb->LSDword ;
            return val;
        }
    ******************************************************************/

    else
        return attr.Raw0_3;
}



void print_smart_log(uint16_t verNo, SmartVendorSpecific attr, int lastAttr)
{
    static uint64_t lsbGbErased = 0, msbGbErased = 0, lsbLifWrtToFlash = 0, msbLifWrtToFlash = 0, lsbLifWrtFrmHost = 0, msbLifWrtFrmHost = 0, lsbLifRdToHost = 0, msbLifRdToHost = 0, lsbTrimCnt = 0, msbTrimCnt = 0;
    char buf[40] = { 0 };
#define NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH 35
    char strBuf[NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH] = { 0 };
    int hideAttr = 0;

    if (attr.AttributeNumber == VS_ATTR_ID_GB_ERASED_LSB)
    {
        lsbGbErased = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_GB_ERASED_MSB)
    {
        msbGbErased = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_LSB)
    {
        lsbLifWrtToFlash = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_MSB)
    {
        msbLifWrtToFlash = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_LSB)
    {
        lsbLifWrtFrmHost = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_MSB) {
        msbLifWrtFrmHost = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_READS_TO_HOST_LSB) {
        lsbLifRdToHost = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_LIFETIME_READS_TO_HOST_MSB)
    {
        msbLifRdToHost = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_TRIM_COUNT_LSB)
    {
        lsbTrimCnt = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if (attr.AttributeNumber == VS_ATTR_ID_TRIM_COUNT_MSB)
    {
        msbTrimCnt = smart_attribute_vs(verNo, attr);
        hideAttr = 1;
    }

    if ((attr.AttributeNumber != 0) && (hideAttr != 1)) {
        printf("%-40s", print_ext_smart_id(attr.AttributeNumber));
        printf("%-15d", attr.AttributeNumber);
        printf(" 0x%016" PRIX64 "", smart_attribute_vs(verNo, attr));
        printf("\n");
    }

    if (lastAttr == 1) {

        snprintf(strBuf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "%s", (print_ext_smart_id(VS_ATTR_ID_GB_ERASED_LSB) + 7));
        printf("%-40s", strBuf);

        printf("%-15d", VS_ATTR_ID_GB_ERASED_MSB << 8 | VS_ATTR_ID_GB_ERASED_LSB);

        snprintf(buf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "0x%016" PRIX64 "%016" PRIX64 "", msbGbErased, lsbGbErased);
        printf(" %s", buf);
        printf("\n");

        snprintf(strBuf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "%s", (print_ext_smart_id(VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_LSB) + 7));
        printf("%-40s", strBuf);

        printf("%-15d", VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_MSB << 8 | VS_ATTR_ID_LIFETIME_WRITES_TO_FLASH_LSB);

        snprintf(buf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "0x%016" PRIX64 "%016" PRIX64, msbLifWrtToFlash, lsbLifWrtToFlash);
        printf(" %s", buf);
        printf("\n");

        snprintf(strBuf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "%s", (print_ext_smart_id(VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_LSB) + 7));
        printf("%-40s", strBuf);

        printf("%-15d", VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_MSB << 8 | VS_ATTR_ID_LIFETIME_WRITES_FROM_HOST_LSB);

        snprintf(buf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "0x%016" PRIX64 "%016" PRIX64, msbLifWrtFrmHost, lsbLifWrtFrmHost);
        printf(" %s", buf);
        printf("\n");

        snprintf(strBuf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "%s", (print_ext_smart_id(VS_ATTR_ID_LIFETIME_READS_TO_HOST_LSB) + 7));
        printf("%-40s", strBuf);

        printf("%-15d", VS_ATTR_ID_LIFETIME_READS_TO_HOST_MSB << 8 | VS_ATTR_ID_LIFETIME_READS_TO_HOST_LSB);

        snprintf(buf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "0x%016" PRIX64 "%016" PRIX64, msbLifRdToHost, lsbLifRdToHost);
        printf(" %s", buf);
        printf("\n");

        snprintf(strBuf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "%s", (print_ext_smart_id(VS_ATTR_ID_TRIM_COUNT_LSB) + 7));
        printf("%-40s", strBuf);
        printf("%-15d", VS_ATTR_ID_TRIM_COUNT_MSB << 8 | VS_ATTR_ID_TRIM_COUNT_LSB);

        snprintf(buf, NVME_PRINT_SMART_LOG_STRING_BUFFER_LENGTH, "0x%016" PRIX64 "%016" PRIX64, msbTrimCnt, lsbTrimCnt);
        printf(" %s", buf);
        printf("\n");
    }
}


void print_smart_log_CF(fb_log_page_CF *pLogPageCF)
{
    uint64_t currentTemp, maxTemp;
    printf("\n\nSeagate DRAM Supercap SMART Attributes :\n");
    printf("%-39s %-19s \n", "Description", "Supercap Attributes");

    printf("%-40s", "Super-cap current temperature");
    currentTemp = pLogPageCF->AttrCF.SuperCapCurrentTemperature;
    /*currentTemp = currentTemp ? currentTemp - 273 : 0;*/
    printf(" 0x%016" PRIX64 "", currentTemp);
    printf("\n");

    maxTemp = pLogPageCF->AttrCF.SuperCapMaximumTemperature;
    /*maxTemp = maxTemp ? maxTemp - 273 : 0;*/
    printf("%-40s", "Super-cap maximum temperature");
    printf(" 0x%016" PRIX64 "", maxTemp);
    printf("\n");

    printf("%-40s", "Super-cap status");
    printf(" 0x%016" PRIX64 "", C_CAST(uint64_t, pLogPageCF->AttrCF.SuperCapStatus));
    printf("\n");

    printf("%-40s", "Data units read to DRAM namespace");
    printf(" 0x%016" PRIX64 "%016" PRIX64 "", pLogPageCF->AttrCF.DataUnitsReadToDramNamespace.MSU64,
        pLogPageCF->AttrCF.DataUnitsReadToDramNamespace.LSU64);
    printf("\n");

    printf("%-40s", "Data units written to DRAM namespace");
    printf(" 0x%016" PRIX64 "%016" PRIX64 "", pLogPageCF->AttrCF.DataUnitsWrittenToDramNamespace.MSU64,
        pLogPageCF->AttrCF.DataUnitsWrittenToDramNamespace.LSU64);
    printf("\n");

    printf("%-40s", "DRAM correctable error count");
    printf(" 0x%016" PRIX64 "", pLogPageCF->AttrCF.DramCorrectableErrorCount);
    printf("\n");

    printf("%-40s", "DRAM uncorrectable error count");
    printf(" 0x%016" PRIX64 "", pLogPageCF->AttrCF.DramUncorrectableErrorCount);
    printf("\n");

}

//Seagate Unique...
int get_Ext_Smrt_Log(tDevice *device)//, nvmeGetLogPageCmdOpts * getLogPageCmdOpts)
{
#ifdef _DEBUG
    printf("-->%s\n", __FUNCTION__);
#endif
    int ret = 0, index = 0;
    EXTENDED_SMART_INFO_T ExtdSMARTInfo;
    memset(&ExtdSMARTInfo, 0x00, sizeof(ExtdSMARTInfo));
    ret = nvme_Read_Ext_Smt_Log(device, &ExtdSMARTInfo);
    if (!ret) {
        printf("%-39s %-15s %-19s \n", "Description", "Ext-Smart-Id", "Ext-Smart-Value");
        for (index = 0; index < 80; index++)
            printf("-");
        printf("\n");
        for (index = 0; index < NUMBER_EXTENDED_SMART_ATTRIBUTES; index++)
            print_smart_log(ExtdSMARTInfo.Version, ExtdSMARTInfo.vendorData[index], index == (NUMBER_EXTENDED_SMART_ATTRIBUTES - 1));

    }
    return 0;

}

int clr_Pcie_Correctable_Errs(tDevice *device)
{
    //const char *desc = "Clear Seagate PCIe Correctable counters for the given device ";
    //const char *save = "specifies that the controller shall save the attribute";
    int err = SUCCESS;

    nvmeFeaturesCmdOpt clearPCIeCorrectableErrors;
    memset(&clearPCIeCorrectableErrors, 0, sizeof(nvmeFeaturesCmdOpt));
    clearPCIeCorrectableErrors.fid = 0xE1;
    clearPCIeCorrectableErrors.featSetGetValue = 0xCB;
    clearPCIeCorrectableErrors.sv = false;
    err = nvme_Set_Features(device, &clearPCIeCorrectableErrors);

    return err;
}
#endif//DISABLE_NVME_PASSTHROUGH
