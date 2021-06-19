TARGET = orgbcli

TEMPLATE = app
CONFIG += console
CONFIG += c++11
CONFIG += static
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -Wno-old-style-cast

INCLUDEPATH += ../../include
INCLUDEPATH += ../../shared/CppUtils-Essential

LIBS += -L../../../build-windows64-release
LIBS += -lorgbsdk
LIBS += -lws2_32

SOURCES += \
	src/CommandRegistration.cpp \
	src/Commands.cpp \
	src/main.cpp

HEADERS += \
	src/CommandRegistration.hpp \
	src/Commands.hpp
