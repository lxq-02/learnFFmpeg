#include "AudioThread.h"
#include "FFmpegs.h"
#include <qDebug>
#include <QFile>

extern "C" {
// 工具相关API（比如错误处理）
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024];  \
    av_strerror(ret, errbuf, sizeof(errbuf));

AudioThread::AudioThread(QObject *parent)
	: QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
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

void AudioThread::run()
{
    AudioDecodeSpec out;
    out.filename = "./out.pcm";

    FFmpegs::aacDecode("./in.aac", out);

    qDebug() << QStringLiteral("采样率:") << out.sampleRate;
    qDebug() << QStringLiteral("采样格式：") << av_get_sample_fmt_name(out.sampleFmt);
    qDebug() << QStringLiteral("声道数：") << av_get_channel_layout_nb_channels(out.chLayout);
}
