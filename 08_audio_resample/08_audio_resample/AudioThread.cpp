#include "AudioThread.h"
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
    const char* inFilename = "D:/44100_s16le_2.pcm";
    QFile  inFile(inFilename);
    const char* outFilename = "D:/48000_f32le_1.pcm";
    QFile  outFile(outFilename);

    // �������
    AVSampleFormat inSampleFmt = AV_SAMPLE_FMT_S16;
    int inSampleRate = 44100;
    int inChLayout = AV_CH_LAYOUT_STEREO;

    // �������
    AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_FLT;
    int outSampleRate = 48000;
    int outChLayout = AV_CH_LAYOUT_MONO;

    // ���ؽ��
    int ret = 0;

    // ָ�����뻺������ָ��
    uint8_t** inData = nullptr;
    // �������Ĵ�С
    int inLinesize = 0;
    // ������
    int inChs = av_get_channel_layout_nb_channels(inChLayout);
    // һ�������Ĵ�С
    int inBytesPerSample = inChs * av_get_bytes_per_sample(inSampleFmt);
    // ����������������
    int inSample = 1024;
    // ��ȡ�ļ����ݵĴ�С
    int len = 0;

    // ָ�������������ָ��
    uint8_t** outData = nullptr;
    // �������Ĵ�С
    int outLinesize = 0;
    // ������
    int outChs = av_get_channel_layout_nb_channels(inChLayout);
    // һ�������Ĵ�С
    int outBytesPerSample = outChs * av_get_bytes_per_sample(outSampleFmt);
    // ����������������
    int outSample = 1024;

    // �����ز���������
    SwrContext* ctx = swr_alloc_set_opts(nullptr, 
        // �������
        outChLayout, outSampleFmt, outSampleRate,
        // �������
        inChLayout, inSampleFmt, inSampleRate,
        0, nullptr);
    if (!ctx)
    {
        qDebug() << "swr_alloc_set_opts error";
        return;
    }

    // ��ʼ���ز���������
    ret = swr_init(ctx);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "swr_init error:" << errbuf;
        goto end;
    }

    // �������뻺����
    ret = av_samples_alloc_array_and_samples(&inData,
        &inLinesize,
        inChs,
        inSample,
        inSampleFmt,
        1);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples errors:" << errbuf;
        goto end;
    }

    // �������������
    ret = av_samples_alloc_array_and_samples(&outData,
        &outLinesize,
        outChs,
        outSample,
        outSampleFmt,
        1);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples errors:" << errbuf;
        goto end;
    }

    // ���ļ�
    if (!inFile.open(QFile::ReadOnly))
    {
        qDebug() << "file open error:" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly))
    {
        qDebug() << "file open error:" << outFilename;
        goto end;
    }

    // ��ȡ�ļ�����
    while ((len = inFile.read((char *)inData[0], inLinesize)) > 0)
    {
        // ��ȡ����������
        inSample = len / inBytesPerSample;
        // �ز���������ֵת���������������
        ret = swr_convert(ctx,
            outData, outSample,
            (const uint8_t **)inData, inSample);

        if (ret < 0)
        {
            ERROR_BUF(ret);
            qDebug() << "swr_convert errors:" << errbuf;
            goto end;
        }

        // ��ת���������д�뵽����ļ���
        outFile.write((char*)outData[0], ret * outBytesPerSample);
    }
    
end:
    // �ͷ���Դ
    // �ر��ļ�
    inFile.close();
    outFile.close();

    // �ͷ����뻺����
    av_freep(&inData);

    // �ͷ����������
    av_freep(&outData);

    // �ͷ��ز���������
    swr_free(&ctx);
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
