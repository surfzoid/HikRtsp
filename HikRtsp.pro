QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
OUTPUT += Console
# You can make your code fail to compile if it uses deprecated APIs.

# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    HikRtsp.cpp \
    h264rtpsource.cpp \
    h265rtpsource.cpp \
    main.cpp \
    rtpparser.cpp \
    settingsform.cpp \
    simplecrypt.cpp

HEADERS += \
    HikRtsp.h \
    h264rtpsource.h \
    h265rtpsource.h \
    include/ApplePlayM4.h \
    include/LinuxPlayM4.h \
    include/PlayM4.h \
    include/WindowsPlayM4.h \
    rtpparser.h \
    settingsform.h \
    simplecrypt.h

FORMS += \
    HikRtsp.ui \
    settingsform.ui

TRANSLATIONS += \
    HikRtsp_en_150.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS +=  -L$$PWD/lib/$$QMAKE_HOST.arch/ -Wl,-rpath=lib/$$QMAKE_HOST.arch:/HCNetSDKCom:./ -lPlayCtrl -lAudioRender -lSuperRender

INCLUDEPATH += $$PWD/lib/x86_64
DEPENDPATH += $$PWD/lib/x86_64
INCLUDEPATH += /usr/include

