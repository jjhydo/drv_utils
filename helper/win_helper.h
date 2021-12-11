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
// \file win_helper.h
// \brief Defines the constants structures specific to Windows OS. 

#pragma once

#include "scsi_helper.h"
#include "sat_helper.h"
#include "common_public.h"
#if !defined(DISABLE_NVME_PASSTHROUGH)
#include "nvme_helper.h"
#endif


#if defined (__cplusplus)
extern "C"
{
#endif


#include <stdlib.h> // for size_t types
#include <stdio.h>  // for printf
#include <string.h> // For memset

#if !defined (__MINGW32__)
#define _NTSCSI_USER_MODE_ //this must be defined before including scsi.h
#include <scsi.h>
#undef _NTSCSI_USER_MODE_
#define INCLUDED_SCSI_DOT_H
#else
#if !defined (SRB_TYPE_SCSI_REQUEST_BLOCK)
#define SRB_TYPE_SCSI_REQUEST_BLOCK 0
#endif
#endif

#define WIN_SCSI_SRB "\\\\.\\SCSI" //can be used to issue mini port ioctls. Not really supported right now...
#define WIN_PHYSICAL_DRIVE  "\\\\.\\PhysicalDrive"
#define WIN_TAPE_DRIVE "\\\\.\\Tape"
#define WIN_CDROM_DRIVE "\\\\.\\CDROM" //Most likely an ATAPI device, but it could be a really old SCSI interface device...
#define WIN_CHANGER_DEVICE "\\\\.\\Changer" //This is a SCSI type device

#define WIN_SCSI_SRB_MAX_LEN UINT8_C(15)
#define WIN_MAX_DEVICE_NAME_LENGTH UINT8_C(40)

#define DOUBLE_BUFFERED_MAX_TRANSFER_SIZE   16384 //Bytes....16KiB to be exact since that is what MS documentation says. - TJE

    //This is the maximum timeout a command can use in Windows...30 hours
#define WIN_MAX_CMD_TIMEOUT_SECONDS 108000

    //If this returns true, a timeout can be sent with INFINITE_TIMEOUT_VALUE definition and it will be issued, otherwise you must try MAX_CMD_TIMEOUT_SECONDS instead
    OPENSEA_TRANSPORT_API bool os_Is_Infinite_Timeout_Supported(void);

    //Configuration manager library is not available on ARM for Windows. Library didn't exist when I went looking for it - TJE
    //NOTE: ARM requires 10.0.16299.0 API to get this library!
#if !defined (__MINGW32__) && !defined (__MINGW64__)
    #pragma comment(lib,"Cfgmgr32.lib")//make sure this get's linked in
#endif
    // \fn send_IO(scsiIoCtx * scsiIoCtx)
    // \brief Function to send a ioctl after converting it from the ScsiIoCtx to OS tSPTIoContext
    // \param ScsiIoCtx
    // \return SUCCESS - pass, !SUCCESS fail or something went wrong
    int send_IO(ScsiIoCtx *scsiIoCtx);

    //-----------------------------------------------------------------------------
    //
    //  os_Device_Reset(tDevice *device)
    //
    //! \brief   Description:  Attempts a device reset through OS functions available. NOTE: This won't work on every device
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //!
    //  Exit:
    //!   \return SUCCESS = pass, OS_COMMAND_NOT_AVAILABLE = not support in this OS or driver of the device, OS_COMMAND_BLOCKED = failed to perform the reset
    //
    //-----------------------------------------------------------------------------
    int os_Device_Reset(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  os_Bus_Reset(tDevice *device)
    //
    //! \brief   Description:  Attempts a bus reset through OS functions available. NOTE: This won't work on every device
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //!
    //  Exit:
    //!   \return SUCCESS = pass, OS_COMMAND_NOT_AVAILABLE = not support in this OS or driver of the device, OS_COMMAND_BLOCKED = failed to perform the reset
    //
    //-----------------------------------------------------------------------------
    int os_Bus_Reset(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  os_Controller_Reset(tDevice *device)
    //
    //! \brief   Description:  Attempts a controller reset through OS functions available. NOTE: This won't work on every device
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //!
    //  Exit:
    //!   \return SUCCESS = pass, OS_COMMAND_NOT_AVAILABLE = not support in this OS or driver of the device, OS_COMMAND_BLOCKED = failed to perform the reset
    //
    //-----------------------------------------------------------------------------
    int os_Controller_Reset(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  os_Lock_Device(tDevice *device)
    //
    //! \brief   Description:  Issues the FSCTL_LOCK_VOLUME ioctl on the open handle to prevent any interuptions during a command or sequence of commands.
    //!                        It is strongly recommended that the unlock is called after this is done to return the device to a "sharing" mode again.
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //  Exit:
    //!   \return SUCCESS = pass, OS_COMMAND_NOT_AVAILABLE = not support in this OS or driver of the device, OS_COMMAND_BLOCKED = failed to perform the reset
    //
    //-----------------------------------------------------------------------------
    OPENSEA_TRANSPORT_API int os_Lock_Device(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  os_Unlock_Device(tDevice *device)
    //
    //! \brief   Description:  Issues the FSCTL_UNLOCK_VOLUME ioctl on the open handle to restore shared functionality on the device.
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //  Exit:
    //!   \return SUCCESS = pass, OS_COMMAND_NOT_AVAILABLE = not support in this OS or driver of the device, OS_COMMAND_BLOCKED = failed to perform the reset
    //
    //-----------------------------------------------------------------------------
    OPENSEA_TRANSPORT_API int os_Unlock_Device(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  os_Unlock_Device(tDevice *device)
    //
    //! \brief   Description:  Issues IOCTL_DISK_UPDATE_PROPERTIES to force an update of the known filesystem...or attempts to.
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //! 
    //  Exit:
    //!   \return SUCCESS = pass, NOT_SUPPORTED = IOCTL not available, or did not work. - TJE
    //
    //-----------------------------------------------------------------------------
    OPENSEA_TRANSPORT_API int os_Update_File_System_Cache(tDevice* device);

#if !defined (DISABLE_NVME_PASSTHROUGH)
    //-----------------------------------------------------------------------------
    //
    //  pci_Read_Bar_Reg()
    //
    //! \brief   Description:  Function to Read PCI Bar register
    //
    //  Entry:
    //!   \param[in]  device = pointer to device context!   
    //!   \param[out] pData =  pointer to data that need to be filled.
    //!                        this needs to be at least the size of a page
    //!                        e.g. getPageSize() in Linux
    //!   \param[out] dataSize =  size of the data
    //! 
    //!
    //  Exit:
    //!   \return SUCCESS = pass, !SUCCESS = something when wrong
    //
    //-----------------------------------------------------------------------------
    int pci_Read_Bar_Reg(tDevice * device, uint8_t * pData, uint32_t dataSize);

    //-----------------------------------------------------------------------------
    //
    //  send_NVMe_IO()
    //
    //! \brief   Description:  Function to send a NVMe command to a device
    //
    //  Entry:
    //!   \param[in] scsiIoCtx = pointer to IO Context!   
    //!
    //  Exit:
    //!   \return SUCCESS = pass, !SUCCESS = something when wrong
    //
    //-----------------------------------------------------------------------------
    int send_NVMe_IO(nvmeCmdCtx *nvmeIoCtx);

    int os_nvme_Reset(tDevice *device);

    int os_nvme_Subsystem_Reset(tDevice *device);

    //-----------------------------------------------------------------------------
    //
    //  getpagesize()
    //
    //! \brief   Description:  Windows equivalent of Linux's getpagesize
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return long page size. 
    //
    //-----------------------------------------------------------------------------
    long getpagesize(void);
#endif

#if defined (__cplusplus)
}
#endif
