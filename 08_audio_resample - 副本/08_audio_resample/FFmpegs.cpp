#include "FFmpegs.h"
#include <QFile>
#include <QDebug>


// 处理错误码
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
    // 文件名
    QFile  inFile(inFilename);
    QFile  outFile(outFilename);

    // 指向输入缓冲区的指针
    uint8_t** inData = nullptr;
    // 缓冲区的大小
    int inLinesize = 0;
    // 声道数
    int inChs = av_get_channel_layout_nb_channels(inChLayout);
    // 一个样本的大小
    int inBytesPerSample = inChs * av_get_bytes_per_sample(inSampleFmt);
    // 缓冲区的样本数量
    int inSample = 1024;
    // 读取文件数据的大小
    int len = 0;

    // 指向输出缓冲区的指针
    uint8_t** outData = nullptr;
    // 缓冲区的大小
    int outLinesize = 0;
    // 声道数
    int outChs = av_get_channel_layout_nb_channels(outChLayout);
    // 一个样本的大小
    int outBytesPerSample = outChs * av_get_bytes_per_sample(outSampleFmt);
    // 缓冲区的样本数量
    int outSample = 1024;

    // 返回结果
    int ret = 0;

    // 创建重采样上下文
    SwrContext* ctx = swr_alloc_set_opts(nullptr,
                                        // 输出参数
                                        outChLayout, outSampleFmt, outSampleRate,
                                        // 输入参数
                                        inChLayout, inSampleFmt, inSampleRate,
                                        0, nullptr);
    if (!ctx)
    {
        qDebug() << "swr_alloc_set_opts error";
        return;
    }

    // 初始化重采样上下文
    ret = swr_init(ctx);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "swr_init error:" << errbuf;
        goto end;
    }

    // 创建输入缓冲区
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

    // 创建输出缓冲区
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

    // 打开文件
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

    // 读取文件数据
    // inData[0] = *inData
    while ((len = inFile.read((char*)inData[0], inLinesize)) > 0)
    {
        // 读取的样本数量
        inSample = len / inBytesPerSample;

        // 重采样（返回值转换后的样本数量）
        ret = swr_convert(ctx,
            outData, outSample,
            (const uint8_t**)inData, inSample);

        if (ret < 0)
        {
            ERROR_BUF(ret);
            qDebug() << "swr_convert errors:" << errbuf;
            goto end;
        }

        // 将转换后的数据写入到输出文件中
        outFile.write((char*)outData[0], ret * outBytesPerSample);
    }

    // 检查一下输出缓冲区是否还有残留的样本（已经重采样过的，转换过的）
    while ((ret = swr_convert(ctx, outData, outSample, nullptr, 0)) > 0) 
    {
        outFile.write((char*)outData[0], ret * outBytesPerSample);
    }

end:
    // 释放资源
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放输入缓冲区
    if (inData)
    {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    // 释放输出缓冲区
    if (outData) 
    {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    // 释放重采样上下文
    swr_free(&ctx);
}
