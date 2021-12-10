//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2019-2021 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
// 
// \file jmicron_nvme_helper.c
// \brief Defines the functions for Jmicron NVMe-USB pass-through

//All code in this file is from a JMicron USB to NVMe product specification for pass-through nvme commands.
//This code should only be used on products that are known to use this pass-through interface.

#if !defined (DISABLE_NVME_PASSTHROUGH)

#include "jmicron_nvme_helper.h"
#include "scsi_helper_func.h" //for ability to send a SCSI IO

int build_JM_NVMe_CDB_And_Payload(uint8_t * cdb, eDataTransferDirection *cdbDataDirection, uint8_t * dataPtr, uint32_t dataSize, eJMNvmeProtocol jmProtocol, eJMNvmeVendorControl jmCtrl, nvmeCmdCtx * nvmCmd)
{
    if (!cdb)
    {
        return BAD_PARAMETER;
    }

    memset(cdb, 0, JMICRON_NVME_CDB_SIZE);

    uint32_t parameterListLength = 0;

    cdb[0] = JMICRON_NVME_PT_OPCODE;
    cdb[1] = C_CAST(uint8_t, jmProtocol);
    cdb[11] = 0;//control byte

    //CDB bytes 3, 4, & 5 are the parameter list length
    //These bytes will be set depending on the size of the transfer for the protocol
    
    switch (jmProtocol)
    {
    case JM_PROTOCOL_SET_PAYLOAD:
        *cdbDataDirection = XFER_DATA_OUT;
        //setup the data payload, which may include a command to issue depending on vendor control field.
        if (!dataPtr || dataSize < JMICRON_NVME_CMD_PAYLOAD_SIZE)
        {
            return BAD_PARAMETER;
        }
        else
        {
            memset(dataPtr, 0, JMICRON_NVME_CMD_PAYLOAD_SIZE);
            parameterListLength = JMICRON_NVME_CMD_PAYLOAD_SIZE;
            //set the signature
            memcpy(dataPtr, JMICRON_NVME_NAMESTRING, strlen(JMICRON_NVME_NAMESTRING));
            //based on vendor ctrl value, we may setup a cmd, or leave those fields blank to setup some other action
            dataPtr[72] = C_CAST(uint8_t, jmCtrl);
            if (jmCtrl == JM_VENDOR_CTRL_SERVICE_PROTOCOL_FIELD)
            {
                //send a cmd
                if (nvmCmd)
                {
                    if (nvmCmd->commandType == NVM_ADMIN_CMD)
                    {
                        //set the admin bit
                        cdb[1] |= BIT7;
                        //Now setup the remaining command fields.
                        //CDW0 is bytes 11:8
                        dataPtr[8] = nvmCmd->cmd.adminCmd.opcode;
                        //TODO: bytes 9, 10, 11 hold fused bits, prp vs sgl, and CID. None of these are filled in for now...-TJE
                        //NSID is 15:12
                        dataPtr[12] = M_Byte0(nvmCmd->cmd.adminCmd.nsid);
                        dataPtr[13] = M_Byte1(nvmCmd->cmd.adminCmd.nsid);
                        dataPtr[14] = M_Byte2(nvmCmd->cmd.adminCmd.nsid);
                        dataPtr[15] = M_Byte3(nvmCmd->cmd.adminCmd.nsid);
                        //metadata ptr is 31:24
                        //TODO: Set metadata ptr value...not sure this is really needed - TJE
                        //data ptr is 47:32
                        //TODO: Set something for the data pointer value? not sure if this is needed today
                        //CDW10 is 51:48
                        dataPtr[48] = M_Byte0(nvmCmd->cmd.adminCmd.cdw10);
                        dataPtr[49] = M_Byte1(nvmCmd->cmd.adminCmd.cdw10);
                        dataPtr[50] = M_Byte2(nvmCmd->cmd.adminCmd.cdw10);
                        dataPtr[51] = M_Byte3(nvmCmd->cmd.adminCmd.cdw10);
                        //CDW11 is 55:52
                        dataPtr[52] = M_Byte0(nvmCmd->cmd.adminCmd.cdw11);
                        dataPtr[53] = M_Byte1(nvmCmd->cmd.adminCmd.cdw11);
                        dataPtr[54] = M_Byte2(nvmCmd->cmd.adminCmd.cdw11);
                        dataPtr[55] = M_Byte3(nvmCmd->cmd.adminCmd.cdw11);
                        //CDW12 is 59:56
                        dataPtr[56] = M_Byte0(nvmCmd->cmd.adminCmd.cdw12);
                        dataPtr[57] = M_Byte1(nvmCmd->cmd.adminCmd.cdw12);
                        dataPtr[58] = M_Byte2(nvmCmd->cmd.adminCmd.cdw12);
                        dataPtr[59] = M_Byte3(nvmCmd->cmd.adminCmd.cdw12);
                        //CDW13 is 63:60
                        dataPtr[60] = M_Byte0(nvmCmd->cmd.adminCmd.cdw13);
                        dataPtr[61] = M_Byte1(nvmCmd->cmd.adminCmd.cdw13);
                        dataPtr[62] = M_Byte2(nvmCmd->cmd.adminCmd.cdw13);
                        dataPtr[63] = M_Byte3(nvmCmd->cmd.adminCmd.cdw13);
                        //CDW14 is 67:64
                        dataPtr[64] = M_Byte0(nvmCmd->cmd.adminCmd.cdw14);
                        dataPtr[65] = M_Byte1(nvmCmd->cmd.adminCmd.cdw14);
                        dataPtr[66] = M_Byte2(nvmCmd->cmd.adminCmd.cdw14);
                        dataPtr[67] = M_Byte3(nvmCmd->cmd.adminCmd.cdw14);
                        //CDW15 is 71:68
                        dataPtr[68] = M_Byte0(nvmCmd->cmd.adminCmd.cdw15);
                        dataPtr[69] = M_Byte1(nvmCmd->cmd.adminCmd.cdw15);
                        dataPtr[70] = M_Byte2(nvmCmd->cmd.adminCmd.cdw15);
                        dataPtr[71] = M_Byte3(nvmCmd->cmd.adminCmd.cdw15);
                    }
                    else
                    {
                        //CDW0 is bytes 11:8
                        cdb[8] = nvmCmd->cmd.nvmCmd.opcode;
                        //TODO: bytes 9, 10, 11 hold fused bits, prp vs sgl, and CID. None of these are filled in for now...-TJE
                        //NSID is 15:12
                        dataPtr[12] = M_Byte0(nvmCmd->cmd.nvmCmd.nsid);
                        dataPtr[13] = M_Byte1(nvmCmd->cmd.nvmCmd.nsid);
                        dataPtr[14] = M_Byte2(nvmCmd->cmd.nvmCmd.nsid);
                        dataPtr[15] = M_Byte3(nvmCmd->cmd.nvmCmd.nsid);
                        //metadata ptr is 31:24
                        //TODO: Set metadata ptr value...not sure this is really needed - TJE
                        //data ptr is 47:32
                        //TODO: Set something for the data pointer value? not sure if this is needed today
                        //CDW10 is 51:48
                        dataPtr[48] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw10);
                        dataPtr[49] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw10);
                        dataPtr[50] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw10);
                        dataPtr[51] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw10);
                        //CDW11 is 55:52
                        dataPtr[52] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw11);
                        dataPtr[53] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw11);
                        dataPtr[54] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw11);
                        dataPtr[55] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw11);
                        //CDW12 is 59:56
                        dataPtr[56] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw12);
                        dataPtr[57] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw12);
                        dataPtr[58] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw12);
                        dataPtr[59] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw12);
                        //CDW13 is 63:60
                        dataPtr[60] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw13);
                        dataPtr[61] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw13);
                        dataPtr[62] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw13);
                        dataPtr[63] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw13);
                        //CDW14 is 67:64
                        dataPtr[64] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw14);
                        dataPtr[65] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw14);
                        dataPtr[66] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw14);
                        dataPtr[67] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw14);
                        //CDW15 is 71:68
                        dataPtr[68] = M_Byte0(nvmCmd->cmd.nvmCmd.cdw15);
                        dataPtr[69] = M_Byte1(nvmCmd->cmd.nvmCmd.cdw15);
                        dataPtr[70] = M_Byte2(nvmCmd->cmd.nvmCmd.cdw15);
                        dataPtr[71] = M_Byte3(nvmCmd->cmd.nvmCmd.cdw15);
                    }
                }
                else
                {
                    return BAD_PARAMETER;
                }
            }
        }
        break;
    case JM_PROTOCOL_DMA_IN:
        *cdbDataDirection = XFER_DATA_IN;
        parameterListLength = dataSize;
        //set admin bit based on CMD being sent
        if (nvmCmd)
        {
            if (nvmCmd->commandType == NVM_ADMIN_CMD)
            {
                //set the admin bit
                cdb[1] |= JMICRON_NVME_ADMIN_BIT;
            }
        }
        else
        {
            return BAD_PARAMETER;
        }
        break;
    case JM_PROTOCOL_DMA_OUT:
        *cdbDataDirection = XFER_DATA_OUT;
        parameterListLength = dataSize;
        //set admin bit based on CMD being sent
        if (nvmCmd)
        {
            if (nvmCmd->commandType == NVM_ADMIN_CMD)
            {
                //set the admin bit
                cdb[1] |= JMICRON_NVME_ADMIN_BIT;
            }
        }
        else
        {
            return BAD_PARAMETER;
        }
        break;
    case JM_PROTOCOL_NON_DATA:
        *cdbDataDirection = XFER_NO_DATA;
        //set admin bit based on CMD being sent
        if (nvmCmd)
        {
            if (nvmCmd->commandType == NVM_ADMIN_CMD)
            {
                //set the admin bit
                cdb[1] |= JMICRON_NVME_ADMIN_BIT;
            }
        }
        else
        {
            return BAD_PARAMETER;
        }
        break;
    case JM_PROTOCOL_RETURN_RESPONSE_INFO:
        *cdbDataDirection = XFER_DATA_IN;
        parameterListLength = JMICRON_NVME_CMD_PAYLOAD_SIZE;
        //TODO: Is setting the admin bit necessary here to match previously sent command???
        if (nvmCmd)
        {
            if (nvmCmd->commandType == NVM_ADMIN_CMD)
            {
                //set the admin bit
                cdb[1] |= JMICRON_NVME_ADMIN_BIT;
            }
        }
        else
        {
            return BAD_PARAMETER;
        }
        break;
    default:
        return BAD_PARAMETER;
    }

    //set parameter list length
    cdb[3] = M_Byte2(parameterListLength);
    cdb[4] = M_Byte1(parameterListLength);
    cdb[5] = M_Byte0(parameterListLength);

    return SUCCESS;
}

int send_JM_NVMe_Cmd(nvmeCmdCtx * nvmCmd)
{
    int ret = SUCCESS;
    uint8_t jmCDB[JMICRON_NVME_CDB_SIZE] = { 0 };
    uint8_t jmPayload[JMICRON_NVME_CMD_PAYLOAD_SIZE] = { 0 };
    eDataTransferDirection jmCDBDir = 0;
    if (!nvmCmd)
    {
        return BAD_PARAMETER;
    }
    //1. build CDB & data for command to send
    //Send CDB to set the command values that will be used to issue a command.
    ret = build_JM_NVMe_CDB_And_Payload(jmCDB, &jmCDBDir, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, JM_PROTOCOL_SET_PAYLOAD, JM_VENDOR_CTRL_SERVICE_PROTOCOL_FIELD, nvmCmd);
    if (SUCCESS != ret)
    {
        return ret;
    }
    ret = scsi_Send_Cdb(nvmCmd->device, jmCDB, JMICRON_NVME_CDB_SIZE, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, jmCDBDir, NULL, 0, 15);
    if (SUCCESS != ret)
    {
        return ret;
    }

    //2. build CDB for data trasfer (including non-data)
    //send CDB for data transfer (this tiggers the actual command to execute)
    eJMNvmeProtocol transferProtocol;
    switch (nvmCmd->commandDirection)
    {
    case XFER_DATA_IN:
        transferProtocol = JM_PROTOCOL_DMA_IN;
        if (nvmCmd->dataSize == 0)
        {
            transferProtocol = JM_PROTOCOL_NON_DATA;
        }
        break;
    case XFER_DATA_OUT:
        transferProtocol = JM_PROTOCOL_DMA_OUT;
        if (nvmCmd->dataSize == 0)
        {
            transferProtocol = JM_PROTOCOL_NON_DATA;
        }
        break;
    case XFER_NO_DATA:
        transferProtocol = JM_PROTOCOL_NON_DATA;
        break;
    default:
        return OS_COMMAND_NOT_AVAILABLE;
    }
    memset(jmCDB, 0, JMICRON_NVME_CDB_SIZE);
    ret = build_JM_NVMe_CDB_And_Payload(jmCDB, &jmCDBDir, NULL, nvmCmd->dataSize, transferProtocol, JM_VENDOR_CTRL_SERVICE_PROTOCOL_FIELD, nvmCmd);
    if (SUCCESS != ret)
    {
        return ret;
    }
    int sendRet = scsi_Send_Cdb(nvmCmd->device, jmCDB, JMICRON_NVME_CDB_SIZE, nvmCmd->ptrData, nvmCmd->dataSize, jmCDBDir, NULL, 0, nvmCmd->timeout);
    //NOTE: do not fail the command or anything else YET.
    //Need to request the response information from the command.
    //TODO: There may be some sense data outputs where the return response info won't work or isn't necessary, but they don't seem documented today. Most likely only for illegal requests.
    bool senseDataIsAllWeGot = true;
    if (sendRet != COMMAND_TIMEOUT)
    {
        //3. build CDB for response info
        //send CDB for response info
        memset(jmCDB, 0, JMICRON_NVME_CDB_SIZE);
        memset(jmPayload, 0, JMICRON_NVME_CMD_PAYLOAD_SIZE);
        ret = build_JM_NVMe_CDB_And_Payload(jmCDB, &jmCDBDir, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, JM_PROTOCOL_RETURN_RESPONSE_INFO, JM_VENDOR_CTRL_SERVICE_PROTOCOL_FIELD, nvmCmd);
        if (SUCCESS == scsi_Send_Cdb(nvmCmd->device, jmCDB, JMICRON_NVME_CDB_SIZE, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, jmCDBDir, NULL, 0, 15))
        {
            //first, check for the NVMe signature to make sure the correct response is here.
            if (0 == memcmp(jmPayload, JMICRON_NVME_NAMESTRING, strlen(JMICRON_NVME_NAMESTRING)))
            {
                senseDataIsAllWeGot = false;
                nvmCmd->commandCompletionData.dw0Valid = true;
                nvmCmd->commandCompletionData.dw3Valid = true;
                nvmCmd->commandCompletionData.commandSpecific = M_BytesTo4ByteValue(jmPayload[11], jmPayload[10], jmPayload[9], jmPayload[8]);
                nvmCmd->commandCompletionData.statusAndCID = M_BytesTo2ByteValue(jmPayload[23], jmPayload[22]) << 17;//only the status field is returned so shift it into the place it's expected to be.
                //All other fields are reserved in the documentation.
            }
        }
    }
    if (senseDataIsAllWeGot)
    {
        nvmCmd->commandCompletionData.dw0Valid = false;
        nvmCmd->commandCompletionData.dw1Valid = false;
        nvmCmd->commandCompletionData.dw2Valid = false;
        nvmCmd->commandCompletionData.dw3Valid = false;
        nvmCmd->commandCompletionData.sqIDandHeadPtr = 0;
        nvmCmd->commandCompletionData.commandSpecific = 0;
        nvmCmd->commandCompletionData.dw1Reserved = 0;
        nvmCmd->commandCompletionData.statusAndCID = 0;
        //Didn't get NVMe response, so no judgements beyond sense data translation can be made.
        ret = sendRet;
    }
    return ret;
}

int jm_NVMe_Normal_Shutdown(tDevice *device)
{
    uint8_t cdb[JMICRON_NVME_CDB_SIZE] = { 0 };
    eDataTransferDirection jmCDBDir = XFER_NO_DATA;
    uint8_t jmPayload[JMICRON_NVME_CMD_PAYLOAD_SIZE] = { 0 };
    int ret = build_JM_NVMe_CDB_And_Payload(cdb, &jmCDBDir, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, JM_PROTOCOL_SET_PAYLOAD, JM_VENDOR_CTRL_NVME_NORMAL_SHUTDOWN, NULL);
    if (ret == SUCCESS)
    {
        ret = scsi_Send_Cdb(device, cdb, JMICRON_NVME_CDB_SIZE, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, jmCDBDir, NULL, 0, 15);
        //TODO: Do we need to do a XFER NO DATA protocol or is this enough to issue this option???
    }
    else
    {
        ret = NOT_SUPPORTED;
    }
    return ret;
}

int jm_NVMe_MCU_Reset(tDevice *device)
{
    uint8_t cdb[JMICRON_NVME_CDB_SIZE] = { 0 };
    eDataTransferDirection jmCDBDir = XFER_NO_DATA;
    uint8_t jmPayload[JMICRON_NVME_CMD_PAYLOAD_SIZE] = { 0 };
    int ret = build_JM_NVMe_CDB_And_Payload(cdb, &jmCDBDir, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, JM_PROTOCOL_SET_PAYLOAD, JM_VENDOR_CTRL_MCU_RESET, NULL);
    if (ret == SUCCESS)
    {
        ret = scsi_Send_Cdb(device, cdb, JMICRON_NVME_CDB_SIZE, jmPayload, JMICRON_NVME_CMD_PAYLOAD_SIZE, jmCDBDir, NULL, 0, 15);
        //TODO: Do we need to do a XFER NO DATA protocol or is this enough to issue this option???
    }
    else
    {
        ret = NOT_SUPPORTED;
    }
    return ret;
}


int jm_nvme_Reset(tDevice *device)
{
    int ret = OS_COMMAND_NOT_AVAILABLE;
    if (device->deviceVerbosity > VERBOSITY_COMMAND_NAMES)
    {
        printf("Sending JMicron NVMe Reset\n");
    }
    if (SUCCESS == jm_NVMe_Normal_Shutdown(device))
    {
        if (SUCCESS == jm_NVMe_MCU_Reset(device))
        {
            ret = SUCCESS;
        }
        else
        {
            ret = FAILURE;
        }
    }
    if (device->deviceVerbosity > VERBOSITY_COMMAND_NAMES)
    {
        print_Return_Enum("Jmicron NVMe Reset", ret);
    }
    return ret;
}

int jm_nvme_Subsystem_Reset(tDevice *device)
{
    int ret = OS_COMMAND_NOT_AVAILABLE;
    if (device->deviceVerbosity > VERBOSITY_COMMAND_NAMES)
    {
        printf("Sending JMicron NVMe Subsystem Reset\n");
    }
    if (SUCCESS == jm_NVMe_Normal_Shutdown(device))
    {
        if (SUCCESS == jm_NVMe_MCU_Reset(device))
        {
            ret = SUCCESS;
        }
        else
        {
            ret = FAILURE;
        }
    }
    if (device->deviceVerbosity > VERBOSITY_COMMAND_NAMES)
    {
        print_Return_Enum("JMicron NVMe Subsystem Reset", ret);
    }
    return ret;
}

#endif //DISABLE_NVME_PASSTHROUGH
