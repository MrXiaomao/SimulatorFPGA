QT -= gui
QT += concurrent

TEMPLATE = lib
DEFINES += ZRSPECTRUMPLUGIN_LIBRARY

CONFIG += c++17
CONFIG += plugin    #告诉构建系统此库是一个插件，并且会生成相应的元数据文件，这是为了能够使用Qt的插件框架功能

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    iplugin.cpp \
    zrspectrumplugin.cpp

HEADERS += \
    iplugin.h \
    qlitethread.h \
    zrspectrumplugin.h

TRANSLATIONS += \
    ZrSpectrumPlugin_zh_CN.ts

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

#指定编译产生的文件分门别类放到对应目录
<<<<<<< HEAD
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj
=======
MOC_DIR     = $$PWD/../../build_SimulatorPro/ZrSpectrumPlugin/temp/moc
RCC_DIR     = $$PWD/../../build_SimulatorPro/ZrSpectrumPlugin/temp/rcc
UI_DIR      = $$PWD/../../build_SimulatorPro/ZrSpectrumPlugin/temp/ui
OBJECTS_DIR = $$PWD/../../build_SimulatorPro/ZrSpectrumPlugin/temp/obj
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

DESTDIR = $$PWD/../../build_SimulatorPro
contains(QT_ARCH, x86_64) {
    # x64
    DESTDIR = $$DESTDIR/x64
} else {
    # x86
    DESTDIR = $$DESTDIR/x86
}

DESTDIR = $$DESTDIR/qt$$QT_VERSION/QtPlugins/simulator
message(DESTDIR = $$DESTDIR)
