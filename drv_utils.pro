QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES  += "WINVER=0x0A00"
DEFINES  += "ENABLE_CSMI=1"
DEFINES  += "STATIC_OPENSEA_OPERATIONS=1"
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    oc/common/common.c \
    oc/common/common_platform.c \
    oc/common/common_windows.c \
    oc/operation/ata_Security.c \
    oc/operation/buffer_test.c \
    oc/operation/defect.c \
    oc/operation/depopulate.c \
    oc/operation/device_statistics.c \
    oc/operation/drive_info.c \
    oc/operation/dst.c \
    oc/operation/firmware_download.c \
    oc/operation/format.c \
    oc/operation/generic_tests.c \
    oc/operation/host_erase.c \
    oc/operation/logs.c \
    oc/operation/nvme_operations.c \
    oc/operation/operations.c \
    oc/operation/power_control.c \
    oc/operation/reservations.c \
    oc/operation/sanitize.c \
    oc/operation/sas_phy.c \
    oc/operation/seagate_operations.c \
    oc/operation/sector_repair.c \
    oc/operation/set_max_lba.c \
    oc/operation/smart.c \
    oc/operation/trim_unmap.c \
    oc/operation/writesame.c \
    oc/operation/zoned_operations.c \
    oc/transport/asmedia_nvme_helper.c \
    oc/transport/ata_cmds.c \
    oc/transport/ata_helper.c \
    oc/transport/ata_legacy_cmds.c \
    oc/transport/cmds.c \
    oc/transport/common_public.c \
    oc/transport/csmi_helper.c \
    oc/transport/csmi_legacy_pt_cdb_helper.c \
    oc/transport/cypress_legacy_helper.c \
    oc/transport/intel_rst_helper.c \
    oc/transport/jmicron_nvme_helper.c \
    oc/transport/nec_legacy_helper.c \
    oc/transport/nvme_cmds.c \
    oc/transport/nvme_helper.c \
    oc/transport/of_nvme_helper.c \
    oc/transport/prolific_legacy_helper.c \
    oc/transport/psp_legacy_helper.c \
    oc/transport/raid_scan_helper.c \
    oc/transport/sat_helper.c \
    oc/transport/sata_helper_func.c \
    oc/transport/scsi_cmds.c \
    oc/transport/scsi_helper.c \
    oc/transport/sntl_helper.c \
    oc/transport/ti_legacy_helper.c \
    oc/transport/usb_hacks.c \
    oc/transport/win_helper.c
HEADERS += \
    mainwindow.h \
    oc/include/common/common.h \
    oc/include/common/common_platform.h \
    oc/include/common/common_windows.h \
    oc/include/common/opensea_common_version.h \
    oc/include/opensea_common_version.h \
    oc/include/operation/ata_Security.h \
    oc/include/operation/buffer_test.h \
    oc/include/operation/defect.h \
    oc/include/operation/depopulate.h \
    oc/include/operation/device_statistics.h \
    oc/include/operation/drive_info.h \
    oc/include/operation/dst.h \
    oc/include/operation/firmware_download.h \
    oc/include/operation/format.h \
    oc/include/operation/generic_tests.h \
    oc/include/operation/host_erase.h \
    oc/include/operation/logs.h \
    oc/include/operation/nvme_operations.h \
    oc/include/operation/opensea_common_version.h \
    oc/include/operation/opensea_operation_version.h \
    oc/include/operation/operations.h \
    oc/include/operation/operations_Common.h \
    oc/include/operation/power_control.h \
    oc/include/operation/reservations.h \
    oc/include/operation/sanitize.h \
    oc/include/operation/sas_phy.h \
    oc/include/operation/seagate_operations.h \
    oc/include/operation/sector_repair.h \
    oc/include/operation/set_max_lba.h \
    oc/include/operation/smart.h \
    oc/include/operation/trim_unmap.h \
    oc/include/operation/writesame.h \
    oc/include/operation/zoned_operations.h \
    oc/include/operation/operations_Common.h \
    oc/include/transport/asmedia_nvme_helper.h \
    oc/include/transport/ata_helper.h \
    oc/include/transport/ata_helper_func.h \
    oc/include/transport/cam_helper.h \
    oc/include/transport/cmds.h \
    oc/include/transport/common_nix.h \
    oc/include/transport/common_public.h \
    oc/include/transport/common_public.h \
    oc/include/transport/common_uefi.h \
    oc/include/transport/common_windows.h \
    oc/include/transport/csmi_helper.h \
    oc/include/transport/csmi_helper_func.h \
    oc/include/transport/csmi_legacy_pt_cdb_helper.h \
    oc/include/transport/csmisas.h \
    oc/include/transport/cypress_legacy_helper.h \
    oc/include/transport/dirent.h \
    oc/include/transport/intel_rst_defs.h \
    oc/include/transport/intel_rst_helper.h \
    oc/include/transport/jmicron_nvme_helper.h \
    oc/include/transport/nec_legacy_helper.h \
    oc/include/transport/nvme_helper.h \
    oc/include/transport/nvme_helper_func.h \
    oc/include/transport/of_nvmeIoctl.h \
    oc/include/transport/of_nvme_helper.h \
    oc/include/transport/of_nvme_helper_func.h \
    oc/include/transport/operations_Common.h \
    oc/include/transport/platform_helper.h \
    oc/include/transport/prolific_legacy_helper.h \
    oc/include/transport/psp_legacy_helper.h \
    oc/include/transport/raid_scan_helper.h \
    oc/include/transport/sat_helper.h \
    oc/include/transport/sat_helper_func.h \
    oc/include/transport/sata_helper_func.h \
    oc/include/transport/sata_types.h \
    oc/include/transport/scsi_helper.h \
    oc/include/transport/scsi_helper_func.h \
    oc/include/transport/sg_helper.h \
    oc/include/transport/sntl_helper.h \
    oc/include/transport/ti_legacy_helper.h \
    oc/include/transport/uefi_helper.h \
    oc/include/transport/usb_hacks.h \
    oc/include/transport/uscsi_helper.h \
    oc/include/transport/version.h \
    oc/include/transport/vm_helper.h \
    oc/include/transport/vm_nvme.h \
    oc/include/transport/vm_nvme_drv_config.h \
    oc/include/transport/vm_nvme_lib.h \
    oc/include/transport/vm_nvme_mgmt.h \
    oc/include/transport/win_helper.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += oc/include/operation
INCLUDEPATH += oc/include/common
INCLUDEPATH += oc/include/transport

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
