QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES  += "WINVER=0x0A00"
DEFINES  += "ENABLE_CSMI=1"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    transport/asmedia_nvme_transport.c \
    transport/ata_cmds.c \
    transport/ata_transport.c \
    transport/ata_legacy_cmds.c \
    transport/cmds.c \
    transport/common.c \
    transport/common_public.c \
    transport/common_windows.c \
    transport/csmi_transport.c \
    transport/csmi_legacy_pt_cdb_transport.c \
    transport/cypress_legacy_transport.c \
    transport/intel_rst_transport.c \
    transport/jmicron_nvme_transport.c \
    transport/nec_legacy_transport.c \
    transport/nvme_cmds.c \
    transport/nvme_transport.c \
    transport/of_nvme_transport.c \
    transport/prolific_legacy_transport.c \
    transport/psp_legacy_transport.c \
    transport/raid_scan_transport.c \
    transport/sat_transport.c \
    transport/sata_transport_func.c \
    transport/scsi_cmds.c \
    transport/scsi_transport.c \
    transport/sntl_transport.c \
    transport/ti_legacy_transport.c \
    transport/usb_hacks.c \
    transport/win_transport.c \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    transport/asmedia_nvme_transport.h \
    transport/ata_transport.h \
    transport/ata_transport_func.h \
    transport/cmds.h \
    transport/common.h \
    transport/common_nix.h \
    transport/common_platform.h \
    transport/common_public.h \
    transport/common_uefi.h \
    transport/common_windows.h \
    transport/csmi_transport.h \
    transport/csmi_transport_func.h \
    transport/csmi_legacy_pt_cdb_transport.h \
    transport/csmisas.h \
    transport/cypress_legacy_transport.h \
    transport/dirent.h \
    transport/intel_rst_defs.h \
    transport/intel_rst_transport.h \
    transport/jmicron_nvme_transport.h \
    transport/nec_legacy_transport.h \
    transport/nvme_transport.h \
    transport/nvme_transport_func.h \
    transport/of_nvmeIoctl.h \
    transport/of_nvme_transport.h \
    transport/of_nvme_transport_func.h \
    transport/opensea_common_version.h \
    transport/platform_transport.h \
    transport/prolific_legacy_transport.h \
    transport/psp_legacy_transport.h \
    transport/raid_scan_transport.h \
    transport/sat_transport.h \
    transport/sat_transport_func.h \
    transport/sata_transport_func.h \
    transport/sata_types.h \
    transport/scsi_transport.h \
    transport/scsi_transport_func.h \
    transport/sntl_transport.h \
    transport/ti_legacy_transport.h \
    transport/usb_hacks.h \
    transport/version.h \
    transport/vm_nvme.h \
    transport/vm_nvme_drv_config.h \
    transport/vm_nvme_mgmt.h \
    transport/win_transport.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
