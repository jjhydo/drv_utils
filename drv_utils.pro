QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES  += "WINVER=0x0A00"
DEFINES  += "ENABLE_CSMI=1"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    helper/asmedia_nvme_helper.c \
    helper/ata_cmds.c \
    helper/ata_helper.c \
    helper/ata_legacy_cmds.c \
    helper/cmds.c \
    helper/common.c \
    helper/common_public.c \
    helper/common_windows.c \
    helper/csmi_helper.c \
    helper/csmi_legacy_pt_cdb_helper.c \
    helper/cypress_legacy_helper.c \
    helper/intel_rst_helper.c \
    helper/jmicron_nvme_helper.c \
    helper/nec_legacy_helper.c \
    helper/nvme_cmds.c \
    helper/nvme_helper.c \
    helper/of_nvme_helper.c \
    helper/prolific_legacy_helper.c \
    helper/psp_legacy_helper.c \
    helper/raid_scan_helper.c \
    helper/sat_helper.c \
    helper/sata_helper_func.c \
    helper/scsi_cmds.c \
    helper/scsi_helper.c \
    helper/sntl_helper.c \
    helper/ti_legacy_helper.c \
    helper/usb_hacks.c \
    helper/win_helper.c \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    helper/asmedia_nvme_helper.h \
    helper/ata_helper.h \
    helper/ata_helper_func.h \
    helper/cmds.h \
    helper/common.h \
    helper/common_nix.h \
    helper/common_platform.h \
    helper/common_public.h \
    helper/common_uefi.h \
    helper/common_windows.h \
    helper/csmi_helper.h \
    helper/csmi_helper_func.h \
    helper/csmi_legacy_pt_cdb_helper.h \
    helper/csmisas.h \
    helper/cypress_legacy_helper.h \
    helper/dirent.h \
    helper/intel_rst_defs.h \
    helper/intel_rst_helper.h \
    helper/jmicron_nvme_helper.h \
    helper/nec_legacy_helper.h \
    helper/nvme_helper.h \
    helper/nvme_helper_func.h \
    helper/of_nvmeIoctl.h \
    helper/of_nvme_helper.h \
    helper/of_nvme_helper_func.h \
    helper/opensea_common_version.h \
    helper/platform_helper.h \
    helper/prolific_legacy_helper.h \
    helper/psp_legacy_helper.h \
    helper/raid_scan_helper.h \
    helper/sat_helper.h \
    helper/sat_helper_func.h \
    helper/sata_helper_func.h \
    helper/sata_types.h \
    helper/scsi_helper.h \
    helper/scsi_helper_func.h \
    helper/sntl_helper.h \
    helper/ti_legacy_helper.h \
    helper/usb_hacks.h \
    helper/version.h \
    helper/vm_nvme.h \
    helper/vm_nvme_drv_config.h \
    helper/vm_nvme_mgmt.h \
    helper/win_helper.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
