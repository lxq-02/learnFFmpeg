
#ifdef _MSC_VER
    //MSVC编译器环境下的代码
    #define _T(str) QString::fromLocal8Bit(str)
#elif __GNUC__
    //MinGW编译器环境下的代码
    #define _T(str) QString(str)
#endif
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# 设置FFmpeg静态库的位置
# -L 搜索路径
# -l 在后面链接库的名字
win32{
FFMPEG_HOME = $$PWD/..
}
INCLUDEPATH += $${FFMPEG_HOME}/include

LIBS += -L$${FFMPEG_HOME}/lib \
        -lavdevice \
        -lavformat \
        -lavutil \
