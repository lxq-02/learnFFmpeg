#include "AudioThread.h"
#include <qDebug>
#include <QFile>

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

#ifdef Q_OS_WIN
// 格式名称
#define FMT_NAME "dshow"
// PCM文件名
#define FILENAME "D:/out.pcm"
#else
#define FMT_NAME "avfoundation"
#define FILENAME "/Users/mj/Destop/out.pcm"
#endif // Q_OS_WIN

AudioThread::AudioThread(QObject *parent)
	: QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

// 当线程启动的时候(start),就会自动调用run函数
// run函数中的代码在子线程中执行
// 耗时操作应该放在run函数中
void AudioThread::run()
{
    qDebug() << QStringLiteral("线程正常开始------------------");
    // 录音逻辑
    // 获取输入格式对象
    AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    // 格式上下文（将来利用上下文操作设备
    AVFormatContext* ctx = nullptr;

#ifdef Q_OS_WIN
    //QString deviceName = QStringLiteral("audio=立体声混音 (Realtek(R) Audio)");
    QString deviceName = QStringLiteral("audio=麦克风阵列 (2- Realtek(R) Audio)");
    QByteArray deviceNameUtf8 = deviceName.toUtf8();
    const char* audio = deviceNameUtf8.constData();
#else
    const char* audio = ":0";
#endif 
    // 打开设备
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

    // 文件名
    QFile file(FILENAME);

    // 打开文件
    // WriteOnly:只写模式。如果文件不存在，创建文件；如果文件存在，清空文件
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << FILENAME << "文件打开失败";

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 数据包
    AVPacket pkt;
    // 不断采集数据
    //while (!_stop && av_read_frame(ctx, &pkt) == 0)
    //{
    //    // 将数据写入文件
    //    file.write((const char*)pkt.data, pkt.size);
    //    qDebug() << "pkt size is :" << pkt.size;
    //}
    while (!isInterruptionRequested() && av_read_frame(ctx, &pkt) == 0)
    {
        // 将数据写入文件
        file.write((const char*)pkt.data, pkt.size);
        qDebug() << "pkt size is :" << pkt.size;
    }


    // 释放资源
    // 关闭文件
    file.close();

    // 关闭设备
    avformat_close_input(&ctx);

    qDebug() << QStringLiteral("线程正常结束------------------");
}

AudioThread::~AudioThread()
{
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();

    qDebug() << this << QStringLiteral("析构(内存被回收)");
}

void AudioThread::setStop(bool stop)
{
    _stop = stop;
}
