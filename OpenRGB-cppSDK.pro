#-------------------------------------------------
#
# Project created by QtCreator 2020-11-01T13:47:35
#
#-------------------------------------------------

QT       -= core gui

TARGET = orgbsdk

TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-old-style-cast -Wno-covered-switch-default

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += include

SOURCES += \
        src/BufferStream.cpp \
        src/Client.cpp \
        src/Color.cpp \
        src/DeviceInfo.cpp \
        src/Protocol.cpp \
        src/Socket.cpp \
        src/SystemErrorInfo.cpp \
        src/Utils.cpp

HEADERS += \
        include/OpenRGB/Client.hpp \
        include/OpenRGB/private/Color.hpp \
        include/OpenRGB/private/DeviceInfo.hpp \
        include/OpenRGB/private/Protocol.hpp \
        include/OpenRGB/private/SystemErrorInfo.hpp \
        src/BufferStream.hpp \
        src/Common.hpp \
        src/Socket.hpp \
        src/Utils.hpp

LIBS += -lws2_32

unix {
    target.path = /usr/lib
    INSTALLS += target
}
