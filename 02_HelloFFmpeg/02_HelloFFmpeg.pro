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

# 设置FFmpeg头文件的位置，以便Qt能够找到它
INCLUDEPATH += D:/DevelopmentTools/ffmpeg-4.3.2-2021-02-27-full_build-shared/include

# 设置FFmpeg静态库的位置
# -L 搜索路径
# -l 在后面链接库的名字
LIBS += -L D:/DevelopmentTools/ffmpeg-4.3.2-2021-02-27-full_build-shared/lib \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lpostproc \
        -lswresample \
        -lswscale
