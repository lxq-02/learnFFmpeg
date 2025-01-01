#include "AudioThread.h"
#include <qDebug>
#include <QFile>

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
#define FILENAME "D:/out.pcm"
#else
#define FMT_NAME "avfoundation"
#define FILENAME "/Users/mj/Destop/out.pcm"
#endif // Q_OS_WIN

AudioThread::AudioThread(QObject *parent)
	: QThread(parent)
{
    // ���������߳̽���ʱ��finished�����͵���deleteLater����
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
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
    else
    {
        qDebug() << QStringLiteral("���豸�ɹ�");
    }

    // �ļ���
    QFile file(FILENAME);

    // ���ļ�
    // WriteOnly:ֻдģʽ������ļ������ڣ������ļ�������ļ����ڣ�����ļ�
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << FILENAME << "�ļ���ʧ��";

        // �ر��豸
        avformat_close_input(&ctx);
        return;
    }

    // ���ݰ�
    AVPacket pkt;
    // ���ϲɼ�����
    //while (!_stop && av_read_frame(ctx, &pkt) == 0)
    //{
    //    // ������д���ļ�
    //    file.write((const char*)pkt.data, pkt.size);
    //    qDebug() << "pkt size is :" << pkt.size;
    //}
    while (!isInterruptionRequested() && av_read_frame(ctx, &pkt) == 0)
    {
        // ������д���ļ�
        file.write((const char*)pkt.data, pkt.size);
        qDebug() << "pkt size is :" << pkt.size;
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
