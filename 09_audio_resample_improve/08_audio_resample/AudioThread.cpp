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

void AudioThread::run()
{
    // 输入参数
    ResampleAudioSpec in;
    in.filename = "H:/44100_s16le_2.pcm";
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.sampleRate = 44100;
    in.chLayout = AV_CH_LAYOUT_STEREO;

    // 输入参数
    ResampleAudioSpec out;
    out.filename = "H:/48000_f32le_1.pcm";
    out.sampleFmt = AV_SAMPLE_FMT_FLT;
    out.sampleRate = 48000;
    out.chLayout = AV_CH_LAYOUT_MONO;

    // 进行音频重采样
    FFmpegs::resampleAudio(in, out);
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
