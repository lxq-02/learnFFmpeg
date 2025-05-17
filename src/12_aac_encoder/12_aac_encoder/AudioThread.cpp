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

void AudioThread::run()
{
    AudioEncodeSpec in;
    in.filename = "H:/44100_s16le_2.pcm";
    in.sampleRate = 44100;
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.chLayout = AV_CH_LAYOUT_STEREO;

    FFmpegs::aacEncode(in, "H:/out.aac");
}
