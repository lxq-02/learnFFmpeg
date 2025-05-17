#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QThread>
#include <QDebug>

extern "C" {
// 设备相关API
#include <libavdevice/avdevice.h>
}

int main(int argc, char *argv[])
{
    qDebug() << "main" << QThread::currentThread();
    // 1、注册设备
    avdevice_register_all();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
