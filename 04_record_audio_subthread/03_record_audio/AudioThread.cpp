#include "AudioThread.h"
#include <qDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
    // �豸���API
#include <libavdevice/avdevice.h>
    // ��ʽ���API
#include <libavformat/avformat.h>
    // �������API�����������
#include <libavutil/avutil.h>
    // �������API
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
// ��ʽ����
#define FMT_NAME "dshow"
// PCM�ļ���
#define FILEPATH "D:/"
#else
#define FMT_NAME "avfoundation"
#define FILEPATH "/Users/mj/Destop/"
#endif // Q_OS_WIN

AudioThread::AudioThread(QObject *parent)
	: QThread(parent)
{
    // ���������߳̽���ʱ��finished�����͵���deleteLater����
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

void showSpec(AVFormatContext* ctx)
{
    // ��ȡ������
    AVStream* stream = ctx->streams[0];
    // ��ȡ��Ƶ����
    AVCodecParameters* params = stream->codecpar;
    // ������
    qDebug() << params->channels;
    // ������
    qDebug() << params->sample_rate;
    // ������ʽ ��С��
    qDebug() << params->format;
    // ÿһ������ռ�ö��ٸ��ֽ�
    qDebug() << "aaa:" << av_get_bytes_per_sample((AVSampleFormat)params->format);
    //av_get_bytes_per_sample
    // ������
    qDebug() << params->channels;

}

// ���߳�������ʱ��(start),�ͻ��Զ�����run����
// run�����еĴ��������߳���ִ��
// ��ʱ����Ӧ�÷���run������
void AudioThread::run()
{
    qDebug() << QStringLiteral("�߳�������ʼ------------------");
    // ¼���߼�
    // ��ȡ�����ʽ����
    AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        qDebug() << "��ȡ�����ʽ����ʧ��" << FMT_NAME;
        return;
    }

    // ��ʽ�����ģ��������������Ĳ����豸
    AVFormatContext* ctx = nullptr;

#ifdef Q_OS_WIN
    //QString deviceName = QStringLiteral("audio=���������� (Realtek(R) Audio)");
    QString deviceName = QStringLiteral("audio=��˷����� (2- Realtek(R) Audio)");
    QByteArray deviceNameUtf8 = deviceName.toUtf8();
    const char* audio = deviceNameUtf8.constData();
#else
    const char* audio = ":0";
#endif 
    // ���豸
    int ret = avformat_open_input(&ctx, audio, fmt, nullptr);
    if (ret < 0)
    {
        char errbuf[1024] = { 0 };
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << QStringLiteral("���豸ʧ��") << errbuf;
        return;
    }

    // ��ӡһ��¼���豸�Ĳ�����Ϣ
    showSpec(ctx);

    // �ļ���
    QString filename = FILEPATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    QFile file(filename);

    // ���ļ�
    // WriteOnly:ֻдģʽ������ļ������ڣ������ļ�������ļ����ڣ�����ļ�
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << filename << "�ļ���ʧ��";

        // �ر��豸
        avformat_close_input(&ctx);
        return;
    }

    // ���ݰ�
    AVPacket pkt;
    while (!isInterruptionRequested())
    {
        // ���ϲɼ�����
        ret = av_read_frame(ctx, &pkt);

        if (ret == 0)   // ��ȡ�ɹ�
        {
            // ������д���ļ�
            file.write((const char*)pkt.data, pkt.size);
        }
        else if (ret == AVERROR(EAGAIN))    // ��Դ��ʱ������
        {
            continue;
        }
        else    // ��������
        {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }


    // �ͷ���Դ
    // �ر��ļ�
    file.close();

    // �ر��豸
    avformat_close_input(&ctx);

    qDebug() << QStringLiteral("�߳���������------------------");
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

void AudioThread::setStop(bool stop)
{
    _stop = stop;
}
