#include "AudioThread.h"
#include <qDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
    // 设备相关API
#include <libavdevice/avdevice.h>
    // 格式相关API
#include <libavformat/avformat.h>
    // 工具相关API（比如错误处理）
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
    // 编码相关API
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
// 格式名称
#define FMT_NAME "dshow"
// PCM文件名
#define FILENAME "./out.yuv"
#define DEVICE_NAME "video=Integrated Camera"
#else
#define FMT_NAME "avfoundation"
#define FILEPATH "/Users/mj/Destop/"
#endif // Q_OS_WIN

#define ERROR_BUF         char errbuf[1024] = { 0 }; \
                            av_strerror(ret, errbuf, sizeof(errbuf));

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

    // 获取输入格式对象
    AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        qDebug() << "av_find_input_format error:" << FMT_NAME;
        return;
    }

    // 格式上下文（将来利用上下文操作设备
    AVFormatContext* ctx = nullptr;

    // 设备参数
    AVDictionary* options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    // 打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << QStringLiteral("打开设备失败") << errbuf;
        return;
    }

    // 文件名
    QFile file(FILENAME);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "file open error:" << FILENAME;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 计算每一帧的大小
    AVCodecParameters* params = ctx->streams[0]->codecpar;
    int imageSize = av_image_get_buffer_size(
        (AVPixelFormat)params->format,
        params->width, params->height,
        1);
    // 数据包
    AVPacket* pkt = av_packet_alloc();
    qDebug() << "Thread interruption requested?" << isInterruptionRequested();
    while (!isInterruptionRequested())
    {
        // 不断采集数据
        ret = av_read_frame(ctx, pkt);

        if (ret == 0)   // 读取成功
        {
            // 将数据写入文件
            file.write((const char*)pkt->data, imageSize);
            /*
             这里要使用imageSize，而不是pkt->size。
             pkt->size有可能比imageSize大（比如在Mac平台），
             使用pkt->size会导致写入一些多余数据到YUV文件中，
             进而导致YUV内容无法正常播放
            */

            // 释放资源
            av_packet_unref(pkt);
        }
        else if (ret == AVERROR(EAGAIN))    // 资源临时不可用
        {
            continue;
        }
        else    // 其他错误
        {
            ERROR_BUF(ret);
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }


    // 释放资源
    // 关闭文件
    file.close();

    av_packet_free(&pkt);

    // 关闭设备
    avformat_close_input(&ctx);

    qDebug() << QStringLiteral("线程正常结束------------------");
}

AudioThread::~AudioThread()
{
    // 断开所有连接
    disconnect();  
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();

    qDebug() << this << QStringLiteral("析构(内存被回收)");
}