#ifdef _MSC_VER
// MSVC编译器环境下的代码
#define _T(str) QString::fromLocal8Bit(str)
#elif __GNUC__
// MinGW编译器环境下的代码
#define _T(str) QString(str)
#endif

#include "MainWindow.h"
#include <qDebug>

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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::on_audioButton_clicked()
{
    // 录音逻辑
    const char* fmtName = "dshow";
    AVInputFormat* fmt = av_find_input_format(fmtName);
    if (!fmt)
    {
        qDebug() << "获取输入格式对象失败" << fmtName;
        return;
    }

    AVFormatContext* ctx = nullptr;
    ctx = avformat_alloc_context();

    // 使用QStringLiteral来处理字符串
    //QString deviceName = QStringLiteral("audio=立体声混音 (Realtek(R) Audio)");
    QString deviceName = QStringLiteral("audio=麦克风阵列 (2- Realtek(R) Audio)");

    // 直接使用UTF-8编码转换为const char*，确保兼容性
    QByteArray deviceNameUtf8 = deviceName.toUtf8();
    const char* audio = deviceNameUtf8.constData();

    int ret = avformat_open_input(&ctx, audio, fmt, nullptr);
    if (ret < 0)
    {
        char errbuf[1024] = { 0 };
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << QStringLiteral("打开设备失败") << errbuf;
        return;
    }
    else
    {
        qDebug() << QStringLiteral("打开设备成功");
    }
}
