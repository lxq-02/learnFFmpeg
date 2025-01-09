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
    AudioDecodeSpec out;
    out.filename = "./out.pcm";

    FFmpegs::aacDecode("./in.aac", out);

    qDebug() << QStringLiteral("������:") << out.sampleRate;
    qDebug() << QStringLiteral("������ʽ��") << av_get_sample_fmt_name(out.sampleFmt);
    qDebug() << QStringLiteral("��������") << av_get_channel_layout_nb_channels(out.chLayout);
}
