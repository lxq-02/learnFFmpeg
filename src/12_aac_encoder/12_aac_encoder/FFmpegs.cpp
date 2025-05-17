#include "FFmpegs.h"
#include <QFile>
#include <QDebug>


// ���������
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs()
{
}

// ��������ʽ
static int check_sample_fmt(const AVCodec* codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat* p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE)
    {
        if (*p == sample_fmt)
        {
            return 1;
        }
        p++;
    }
    return 0;
}

// ��Ƶ����
// ���ظ�������;�����˴���
// ����0���ɹ�
static int encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, QFile& outFile)
{
    // �������ݵ�������
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error " << errbuf;
        return ret;
    }

    // ���ϴӱ������л�ȡ����������
    while (true)
    {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // ������ȡ���ݵ�frame��Ȼ���͵�������
            return 0;
        }
        else if (ret < 0)   // ��������
        {
            return ret;
        }

        // �ɹ���ȡ���������ݰ���д���ļ�
        // ���ﲻ��ֱ�Ӳ��� outFile����Ϊ pkt �Ǳ�������������ݽṹ���������������ݺ�Ԫ��Ϣ������ͨ�� pkt ������ȷ��ȡ���ݡ�
        outFile.write((char *)pkt->data, pkt->size);

        // �ͷ�pkt�ڲ�����Դ
        av_packet_unref(pkt);
    }
}

// ����
void FFmpegs::aacEncode(AudioEncodeSpec& in, const char* outFilename)
{
    // �ļ�
    QFile inFile(in.filename);
    QFile outFile(outFilename);

    // ���ؽ��
    int ret = 0;

    // ������
    AVCodec* codec = nullptr;

    // ����������
    AVCodecContext* ctx = nullptr;

    // ��ű���ǰ�����ݣ�pcm��
    AVFrame* frame = nullptr;

    // ��ű��������ݣ�aac��
    AVPacket* pkt = nullptr;

    // ��ȡ������
    //codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec)
    {
        qDebug() << "encoder not found";
        return;
    }

    // libfdk_aac���������ݵ�Ҫ�󣺲�����ʽ������16λ����
    if (!check_sample_fmt(codec, in.sampleFmt))
    {
        qDebug() << "unsupported sample format" << av_get_sample_fmt_name(in.sampleFmt);
        return;
    }

    // ��������������
    ctx = avcodec_alloc_context3(codec);
    if (!ctx)
    {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    // ����PCM����
    ctx->sample_rate = in.sampleRate;
    ctx->sample_fmt = in.sampleFmt;
    ctx->channel_layout = in.chLayout;
    //// ������
    //ctx->bit_rate = 32000;
    //// ���
    //ctx->profile = FF_PROFILE_AAC_HE_V2;

    // �򿪱�����
    //AVDictionary* options = nullptr;
    //av_dict_set(&options, "vbr", "1", 0);
    //ret = avcodec_open2(ctx, codec, &options);
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // ����AVFrame
    frame = av_frame_alloc();
    if (!frame)
    {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // frame�������е�����֡��������ctx->frmae_size����)
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->channel_layout = ctx->channel_layout;

    // ����nb_samples��format��channel_layout����������
    ret = av_frame_get_buffer(frame, 0);
    if (ret)
    {
        ERROR_BUF(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    // ����AVPacket
	pkt = av_packet_alloc();
	if (!pkt)
	{
		qDebug() << "av_packet_alloc error";
		goto end;
	}

    // ���ļ�
    if (!inFile.open(QFile::ReadOnly))
	{
		qDebug() << "open file error" << in.filename;
		goto end;
	}
	if (!outFile.open(QFile::WriteOnly))
	{
		qDebug() << "open file error" << outFilename;
		goto end;
	}

    // ��ȡ���ݵ�frame��
    while ((ret = inFile.read((char*)frame->data[0], frame->linesize[0])) > 0)
    {
        // ���ļ��ж�ȡ�����ݣ�����������frame������
        if (ret < frame->linesize[0])
        {
            int bytes = av_get_bytes_per_sample((AVSampleFormat)frame->format);
            int ch = av_get_channel_layout_nb_channels(frame->channel_layout);
            // ����������Ч������֡����
            // ��ֹ������������һЩ��������
            frame->nb_samples = ret / (bytes * ch);
        }

        // ���б���
        if (encode(ctx, frame, pkt, outFile) < 0)
        {
            goto end;
        }
    }

    // ˢ�»�����
    encode(ctx, nullptr, pkt, outFile);

end:
    // �ر��ļ�
    inFile.close();
    outFile.close();

    // �ͷ���Դ
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
    qDebug() << QStringLiteral("�߳���������");
}
