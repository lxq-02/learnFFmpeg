#include "FFmpegs.h"
#include <QFile>
#include <QDebug>


// ���������
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

void FFmpegs::resampleAudio(ResampleAudioSpec& in, ResampleAudioSpec& out)
{
    resampleAudio(in.filename, in.sampleRate, in.sampleFmt, in.chLayout, 
                  out.filename, out.sampleRate, out.sampleFmt, out.chLayout);
}

void FFmpegs::resampleAudio(const char* inFilename, int inSampleRate, AVSampleFormat inSampleFmt, int inChLayout, const char* outFilename, int outSampleRate, AVSampleFormat outSampleFmt, int outChLayout)
{
    // �ļ���
    QFile  inFile(inFilename);
    QFile  outFile(outFilename);

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
    int outChs = av_get_channel_layout_nb_channels(outChLayout);
    // һ�������Ĵ�С
    int outBytesPerSample = outChs * av_get_bytes_per_sample(outSampleFmt);
    // ����������������
    int outSample = 1024;

    // ���ؽ��
    int ret = 0;

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
    // inData[0] = *inData
    while ((len = inFile.read((char*)inData[0], inLinesize)) > 0)
    {
        // ��ȡ����������
        inSample = len / inBytesPerSample;

        // �ز���������ֵת���������������
        ret = swr_convert(ctx,
            outData, outSample,
            (const uint8_t**)inData, inSample);

        if (ret < 0)
        {
            ERROR_BUF(ret);
            qDebug() << "swr_convert errors:" << errbuf;
            goto end;
        }

        // ��ת���������д�뵽����ļ���
        outFile.write((char*)outData[0], ret * outBytesPerSample);
    }

    // ���һ������������Ƿ��в������������Ѿ��ز������ģ�ת�����ģ�
    while ((ret = swr_convert(ctx, outData, outSample, nullptr, 0)) > 0) 
    {
        outFile.write((char*)outData[0], ret * outBytesPerSample);
    }

end:
    // �ͷ���Դ
    // �ر��ļ�
    inFile.close();
    outFile.close();

    // �ͷ����뻺����
    if (inData)
    {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    // �ͷ����������
    if (outData) 
    {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    // �ͷ��ز���������
    swr_free(&ctx);
}
