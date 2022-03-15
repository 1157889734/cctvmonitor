QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = cctvtest

QMAKE_CFLAGS += $(STRIP) $(TAR_FILE)
QMAKE_CXXFLAGS += $(STRIP) $(TAR_FILE)

QMAKE_CFLAGS += -g
QMAKE_CXXFLAGS += -g

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/includeVdecc

#INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/rockchip/
##INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/libdrm
#INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/rga/

LIBS += -L$$PWD/lib/ -lrkcmplay
#LIBS += -lrga -lrockchip_mpp -lwayland-client

LIBS += -L$$PWD/lib/  -ldl -lz -lbz2 -lrockchip_mpp -lrga -lpthread -lwayland-client

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cctv/NVRMsgProc.cpp \
    cctv/cctv.cpp \
    cctv/comm.cpp \
    cctv/main.cpp \
    cctv/menuwidget.cpp \
    cctv/mutex.cpp \
    cctv/myslider.cpp \
    cctv/recordmanage.cpp \
    cctv/sysmanage.cpp \
    cctv/timeset.cpp \
    debugout/debug.c \
    ftp/ftpApi.c \
    ftp/gb2312_utf8.c \
    log/log.c \
    pmsg/multicast.c \
    pmsg/pmsgcli.c \
    pmsg/pmsgproc.c \
    state/fileConfig.c \
    state/state.c \

HEADERS += \
    cctv/NVRMsgProc.h \
    cctv/cctv.h \
    cctv/comm.h \
    cctv/menuwidget.h \
    cctv/mutex.h \
    cctv/myslider.h \
    cctv/recordmanage.h \
    cctv/sysmanage.h \
    cctv/timeset.h \
    cctv/types.h \
    debugout/debug.h \
    ftp/ftpApi.h \
    ftp/gb2312_utf8.h \
    include/CMPlayerInterface.h \
    include/debug.h \
    include/mutex.h \
    include/rtspApi.h \
    include/rtspComm.h \
    include/vdec.h \
    log/log.h \
    pmsg/multicast.h \
    pmsg/pmsgcli.h \
    pmsg/pmsgproc.h \
    state/fileConfig.h \
    state/state.h \


FORMS += \
    cctv/cctv.ui \
    cctv/menuwidget.ui \
    cctv/recordmanage.ui \
    cctv/sysmanage.ui \
    cctv/timeset.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc


