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
#include <libavutil/imgutils.h>
    // �������API
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
// ��ʽ����
#define FMT_NAME "dshow"
// PCM�ļ���
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
    // ���������߳̽���ʱ��finished�����͵���deleteLater����
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
} 

// ���߳�������ʱ��(start),�ͻ��Զ�����run����
// run�����еĴ��������߳���ִ��
// ��ʱ����Ӧ�÷���run������
void AudioThread::run()
{
    qDebug() << QStringLiteral("�߳�������ʼ------------------");

    // ��ȡ�����ʽ����
    AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        qDebug() << "av_find_input_format error:" << FMT_NAME;
        return;
    }

    // ��ʽ�����ģ��������������Ĳ����豸
    AVFormatContext* ctx = nullptr;

    // �豸����
    AVDictionary* options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    // ���豸
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << QStringLiteral("���豸ʧ��") << errbuf;
        return;
    }

    // �ļ���
    QFile file(FILENAME);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "file open error:" << FILENAME;

        // �ر��豸
        avformat_close_input(&ctx);
        return;
    }

    // ����ÿһ֡�Ĵ�С
    AVCodecParameters* params = ctx->streams[0]->codecpar;
    int imageSize = av_image_get_buffer_size(
        (AVPixelFormat)params->format,
        params->width, params->height,
        1);
    // ���ݰ�
    AVPacket* pkt = av_packet_alloc();
    qDebug() << "Thread interruption requested?" << isInterruptionRequested();
    while (!isInterruptionRequested())
    {
        // ���ϲɼ�����
        ret = av_read_frame(ctx, pkt);

        if (ret == 0)   // ��ȡ�ɹ�
        {
            // ������д���ļ�
            file.write((const char*)pkt->data, imageSize);
            /*
             ����Ҫʹ��imageSize��������pkt->size��
             pkt->size�п��ܱ�imageSize�󣨱�����Macƽ̨����
             ʹ��pkt->size�ᵼ��д��һЩ�������ݵ�YUV�ļ��У�
             ��������YUV�����޷���������
            */

            // �ͷ���Դ
            av_packet_unref(pkt);
        }
        else if (ret == AVERROR(EAGAIN))    // ��Դ��ʱ������
        {
            continue;
        }
        else    // ��������
        {
            ERROR_BUF(ret);
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }


    // �ͷ���Դ
    // �ر��ļ�
    file.close();

    av_packet_free(&pkt);

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