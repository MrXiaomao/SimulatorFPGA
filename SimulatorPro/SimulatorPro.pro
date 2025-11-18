QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050E00

SOURCES += \
    clientwindow.cpp \
    createsessionwindow.cpp \
    globalsettings.cpp \
    main.cpp \
    mainwindow.cpp \
    serverwindow.cpp

HEADERS += \
    clientwindow.h \
    createsessionwindow.h \
    globalsettings.h \
    mainwindow.h \
    qlitethread.h \
    serverwindow.h

FORMS += \
    clientwindow.ui \
    createsessionwindow.ui \
    mainwindow.ui \
    serverwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#RC_ICONS = "icons\ico.ico"

exists (./.git) {
    GIT_BRANCH   = $$system(git rev-parse --abbrev-ref HEAD)
    GIT_DATE     = $$system(git show --oneline --format=\"%ci\" -s HEAD)
    GIT_HASH     = $$system(git show --oneline --format=\"%H\" -s HEAD)
    GIT_VERSION = "Git: $${GIT_BRANCH}: $${GIT_DATE} $${GIT_HASH}"
} else {
    GIT_BRANCH      = None
    GIT_DATE        = None
    GIT_HASH        = None
    GIT_VERSION     = None
}

DEFINES += GIT_BRANCH=\"\\\"$$GIT_BRANCH\\\"\"
DEFINES += GIT_DATE=\"\\\"$$GIT_DATE\\\"\"
DEFINES += GIT_HASH=\"\\\"$$GIT_HASH\\\"\"
DEFINES += GIT_VERSION=\"\\\"$$GIT_VERSION\\\"\"
DEFINES += APP_VERSION="\\\"V1.0.1\\\""
#DEFINES += ENABLE_LIBHV
#DEFINES += ENABLE_LIBHV=1

win32 {
    # libhv
    contains(DEFINES, ENABLE_LIBHV) {
        message("Including libhv library")
        INCLUDEPATH += $$PWD/../3rdParty/libhv/include
        LIBS += -L$$PWD/../3rdParty/libhv/lib -lhv
        LIBS += -lws2_32
    } else {
        message("Skip libhv library")
    }
}

windows {
    # MinGW
    *-g++* {
        QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic -Wdeprecated-declarations
        QMAKE_CXXFLAGS += -Wdeprecated-declarations
        #QMAKE_CXXFLAGS += -finput-charset=UTF-8
        #QMAKE_CXXFLAGS += -fexec-charset=UTF-8
        #QMAKE_CXXFLAGS += -fwide-exec-charset=UTF-16
        #设置wchar_t类型数据的编码格式。不同主机值可能不同，编译器运行时根据主机情况会自动识别出最符合
        #主机的方案作为默认值，这个参数是不需要动的。UTF-16 UTF-16BE UTF-16LE UTF-32LE UTF-32BE
    }
    # MSVC
    *-msvc* {
        QMAKE_CXXFLAGS += /utf-8
        QMAKE_CXXFLAGS += /source-charset:utf-8
        QMAKE_CXXFLAGS += /execution-charset:utf-8
    }
}

#指定编译产生的文件分门别类放到对应目录
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj

include($$PWD/IPlugin/Plugin.pri)
include($$PWD/../../3rdParty/resource/resource.pri)
include($$PWD/../../3rdParty/log4qt/Include/log4qt.pri)
#include($$PWD/../../3rdParty/QFontIcon/QFontIcon.pri)
include($$PWD/../../3rdParty/hdf5/C++/hdf5Wrapper.pri)
include($$PWD/../../3rdParty/QGoodWindow/QGoodWindow/QGoodWindow.pri)
include($$PWD/../../3rdParty/QGoodWindow/QGoodCentralWidget/QGoodCentralWidget.pri)

DESTDIR = $$PWD/../../build_SimulatorPro
contains(QT_ARCH, x86_64) {
    # x64
    DESTDIR = $$DESTDIR/x64
} else {
    # x86
    DESTDIR = $$DESTDIR/x86
}

DESTDIR = $$DESTDIR/qt$$QT_VERSION/
message(DESTDIR = $$DESTDIR)

#开启工程的debug和release两种版本构建
#CONFIG += debug_and_release
#避免创建空的debug和release目录
CONFIG -= debug_and_release
CONFIG(debug, debug|release) {
    TARGET = SimulatorProd
} else {
    TARGET = SimulatorPro
}

RESOURCES += \
    lresource.qrc
