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
    AudioEncodeSpec in;
    in.filename = "H:/44100_s16le_2.pcm";
    in.sampleRate = 44100;
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.chLayout = AV_CH_LAYOUT_STEREO;

    FFmpegs::aacEncode(in, "H:/out.aac");
}
