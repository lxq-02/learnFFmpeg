#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <Windows.h>

extern "C" {
// 设备相关API
#include <libavdevice/avdevice.h>
// 格式相关API
#include <libavformat/avformat.h>
// 工具相关API（比如错误处理）
#include <libavutil/avutil.h>
// 编码相关API
#include <libavcodec/avcodec.h>
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_audioButton_clicked()
{
    // 录音逻辑
    // 2、获取输入格式对象
    // dshow就是输入格式，输入设备的格式
    // ffmpeg -f dshow -list_devices
    const char* fmtName = "dshow";
    AVInputFormat *fmt = av_find_input_format(fmtName);
    if (!fmt)
    {
        qDebug() << "获取输入格式对象失败" << fmtName;
        return;
    }

    AVFormatContext *ctx = nullptr;

    // 3、打开设备
    // 使用 QString 来处理中文字符串
    QString deviceName = QString::fromUtf8("video=麦克风阵列（2- Realtek(R) Audio)");

    qDebug() << deviceName;
    // 将 QString 转换为 const char*
    std::string deviceNameStr = deviceName.toStdString();

    int ret = avformat_open_input(&ctx, deviceNameStr.c_str(), fmt, nullptr);
    // 如果打开设备失败
    if (ret < 0)
    {
        char errbuf[1024] = {0};
        // 根据函数返回的错误码获取错误信息
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }
    else
    {
        qDebug() << "打开设备成功";
    }
}
