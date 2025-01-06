#include "AudioThread.h"
#include "FFmpegs.h"
#include <qDebug>
#include <QFile>

extern "C" {

    // �������API�����������
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024];  \
    av_strerror(ret, errbuf, sizeof(errbuf));

AudioThread::AudioThread(QObject *parent)
	: QThread(parent)
{
    // ���������߳̽���ʱ��finished�����͵���deleteLater����
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

void AudioThread::run()
{
    // �������
    ResampleAudioSpec in;
    in.filename = "H:/44100_s16le_2.pcm";
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.sampleRate = 44100;
    in.chLayout = AV_CH_LAYOUT_STEREO;

    // �������
    ResampleAudioSpec out;
    out.filename = "H:/48000_f32le_1.pcm";
    out.sampleFmt = AV_SAMPLE_FMT_FLT;
    out.sampleRate = 48000;
    out.chLayout = AV_CH_LAYOUT_MONO;

    // ������Ƶ�ز���
    FFmpegs::resampleAudio(in, out);
}

AudioThread::~AudioThread()
{
    // �Ͽ���������
    disconnect();  
    // �ڴ����֮ǰ�����������߳�
    requestInterruption();
    // ��ȫ�˳�
    quit();
    wait();

    qDebug() << this << QStringLiteral("����(�ڴ汻����)");
}
