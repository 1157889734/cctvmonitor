QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


QMAKE_CFLAGS += $(STRIP) $(TAR_FILE)
QMAKE_CXXFLAGS += $(STRIP) $(TAR_FILE)

#QMAKE_CFLAGS += -g
#QMAKE_CXXFLAGS += -g

INCLUDEPATH += $$PWD/include
INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/rockchip/
#INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/libdrm
INCLUDEPATH += /home/cftc/toolchain/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/rga/
LIBS += -L$$PWD/lib/  -ldl -lz -lbz2 -lrockchip_mpp -lrga -lpthread -lwayland-client

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    NVRMsgProc.cxx \
    debugout/debug.c \
    ftp/ftpApi.c \
    ftp/gb2312_utf8.c \
    log/log.c \
    main.cpp \
    cctvtest.cpp \
    mainforn.cpp \
    mutex.cpp \
    playwidget.cpp \
    pmsg/multicast.c \
    pmsg/pmsgcli.c \
    pmsg/pmsgproc.c \
    recordmanage.cpp \
    rtsp/Base64EncDec.c \
    rtsp/md5.c \
    rtsp/ourMD5.c \
    rtsp/rtcp.c \
    rtsp/rtp.c \
    rtsp/rtsp.c \
    rtsp/rtspApi.c \
    rtsp/rtspComm.c \
    state/fileConfig.c \
    state/state.c \
    sysmanage.cpp \
    vdec/cmplayer.cpp \
    vdec/rgaapi.cpp \
    vdec/shm.cpp \
    vdec/vdec.cpp

HEADERS += \
    NVRMsgProc.h \
    cctvtest.h \
    debugout/debug.h \
    ftp/ftpApi.h \
    ftp/gb2312_utf8.h \
    log/log.h \
    mainforn.h \
    mutex.h \
    playwidget.h \
    pmsg/multicast.h \
    pmsg/pmsgcli.h \
    pmsg/pmsgproc.h \
    recordmanage.h \
    rtsp/Base64EncDec.h \
    rtsp/md5.h \
    rtsp/mutex.h \
    rtsp/ourMD5.h \
    rtsp/rtcp.h \
    rtsp/rtp.h \
    rtsp/rtsp.h \
    rtsp/rtspApi.h \
    rtsp/rtspComm.h \
    rtsp/types.h \
    state/fileConfig.h \
    state/state.h \
    sysmanage.h \
    types.h \
    vdec/cmplayer.h \
    vdec/rga_api.h \
    vdec/rgaapi.h \
    vdec/shm.h \
    vdec/vdec.h

FORMS += \
    cctvtest.ui \
    mainforn.ui \
    recordmanage.ui \
    sysmanage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    rtsp/Base64EncDec.o \
    rtsp/Makefile \
    rtsp/Makefile.bak \
    rtsp/librtsp.a \
    rtsp/md5.o \
    rtsp/ourMD5.o \
    rtsp/rtcp.o \
    rtsp/rtp.o \
    rtsp/rtsp.h.bak \
    rtsp/rtsp.o \
    rtsp/rtspApi.o \
    rtsp/rtspComm.o
