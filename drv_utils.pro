QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES  += "WINVER=0x0A00"
DEFINES  += "ENABLE_CSMI=1"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    transport/asmedia_nvme_helper.c \
    transport/ata_cmds.c \
    transport/ata_helper.c \
    transport/ata_legacy_cmds.c \
    transport/cmds.c \
    transport/common.c \
    transport/common_public.c \
    transport/common_windows.c \
    transport/csmi_helper.c \
    transport/csmi_legacy_pt_cdb_helper.c \
    transport/cypress_legacy_helper.c \
    transport/intel_rst_helper.c \
    transport/jmicron_nvme_helper.c \
    transport/nec_legacy_helper.c \
    transport/nvme_cmds.c \
    transport/nvme_helper.c \
    transport/of_nvme_helper.c \
    transport/prolific_legacy_helper.c \
    transport/psp_legacy_helper.c \
    transport/raid_scan_helper.c \
    transport/sat_helper.c \
    transport/sata_helper_func.c \
    transport/scsi_cmds.c \
    transport/scsi_helper.c \
    transport/sntl_helper.c \
    transport/ti_legacy_helper.c \
    transport/usb_hacks.c \
    transport/win_helper.c \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    transport/asmedia_nvme_helper.h \
    transport/ata_helper.h \
    transport/ata_helper_func.h \
    transport/cmds.h \
    transport/common.h \
    transport/common_nix.h \
    transport/common_platform.h \
    transport/common_public.h \
    transport/common_uefi.h \
    transport/common_windows.h \
    transport/csmi_helper.h \
    transport/csmi_helper_func.h \
    transport/csmi_legacy_pt_cdb_helper.h \
    transport/csmisas.h \
    transport/cypress_legacy_helper.h \
    transport/dirent.h \
    transport/intel_rst_defs.h \
    transport/intel_rst_helper.h \
    transport/jmicron_nvme_helper.h \
    transport/nec_legacy_helper.h \
    transport/nvme_helper.h \
    transport/nvme_helper_func.h \
    transport/of_nvmeIoctl.h \
    transport/of_nvme_helper.h \
    transport/of_nvme_helper_func.h \
    transport/opensea_common_version.h \
    transport/platform_helper.h \
    transport/prolific_legacy_helper.h \
    transport/psp_legacy_helper.h \
    transport/raid_scan_helper.h \
    transport/sat_helper.h \
    transport/sat_helper_func.h \
    transport/sata_helper_func.h \
    transport/sata_types.h \
    transport/scsi_helper.h \
    transport/scsi_helper_func.h \
    transport/sntl_helper.h \
    transport/ti_legacy_helper.h \
    transport/usb_hacks.h \
    transport/version.h \
    transport/vm_nvme.h \
    transport/vm_nvme_drv_config.h \
    transport/vm_nvme_mgmt.h \
    transport/win_helper.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
