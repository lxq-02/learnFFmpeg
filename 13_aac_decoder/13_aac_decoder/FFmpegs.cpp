#include "FFmpegs.h"
#include <QFile>
#include <QDebug>

// ���뻺�����Ĵ�С
#define IN_DATA_SIZE 20480
// ��Ҫ�ٴζ�ȡ�����ļ����ݵ���ֵ
#define REFILL_THRESH 4096

// ���������
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs()
{
}

static int decode(AVCodecContext* ctx,
    AVPacket* pkt,
    AVFrame* frame,
    QFile& outFile)
{
    // ����ѹ�����ݵ�������
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_packet error:" << errbuf;
        return ret;
    }

    while (true)
    {
        // ��ȡ����������
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return 0;
        }
        else if (ret < 0)
        {
            ERROR_BUF(ret);
            qDebug() << "avcodec_receive_frame error:" << errbuf;
            return ret;
        }
        // ������������д���ļ�
        outFile.write((char *)frame->data[0], frame->linesize[0]);
    }
}

void FFmpegs::aacDecode(const char* inFilename, AudioDecodeSpec& out)
{
    // ���ؽ��
    int ret = 0;

    // ������Ŷ�ȡ���ļ�����(aac)
    // ����AV_INPUT_BUFFER_PADDING_SIZE��ԭ����Ϊ�˷�ֹĳЩ�Ż�����readerһ���Զ�ȡ���ർ��Խ��
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char* inData = inDataArray;

    // ÿ�δ������ļ��ж�ȡ�ĳ��ȣ�aac��
    int inLen;
    // �Ƿ��Ѿ���ȡ���������ļ���ĩβ
    int inEnd = 0;

    // �ļ�
    QFile inFile(inFilename);
    QFile outFile(out.filename);

    // ������
    AVCodec* codec = nullptr;
    // ������
    AVCodecContext* ctx = nullptr;
    // ������������
    AVCodecParserContext* parserCtx = nullptr;

    // ��Ž���ǰ������(aac)
    AVPacket* pkt = nullptr;
    // ��Ž���������(pcm)
    AVFrame* frame = nullptr;

    // ��ȡ������
    codec = avcodec_find_decoder_by_name("libfdk_aac");
    if (!codec)
    {
        qDebug() << "decoder not found";
        return;
    }

    // ��ʼ��������������
    parserCtx = av_parser_init(codec->id);
    if (!parserCtx)
    {
        qDebug() << "av_parser_init error";
        return;
    }

    // ����������
    ctx = avcodec_alloc_context3(codec);
    if (!ctx)
    {
        qDebug() << "avcodec_alloc_context3 error";
        goto end;
    }

    // ����AVPacket
    pkt = av_packet_alloc();
    if (!pkt)
    {
        qDebug() << "av_packet_alloc error";
        goto end;
	}
    
    // ����AVFrame
	frame = av_frame_alloc();
	if (!frame)
	{
		qDebug() << "av_frame_alloc error";
		goto end;
	}

    // �򿪽�����
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error: " << errbuf;
        goto end;
    }

    // ���ļ�
    if (!inFile.open(QFile::ReadOnly))
	{
		qDebug() << "file open error:" << inFilename;
		goto end;
	}

	// ������ļ�
    if (!outFile.open(QFile::WriteOnly))
    {
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    // ��ȡ�ļ�����
    inLen = inFile.read(inData, IN_DATA_SIZE);
    while (inLen > 0)
    {
        // ���������������Ĵ���
        ret = av_parser_parse2(parserCtx, ctx, 
                                &pkt->data, &pkt->size, 
                                (uint8_t*)inData, inLen, 
                                AV_NOPTS_VALUE,     AV_NOPTS_VALUE, 0);
        if (ret < 0)
        {
            ERROR_BUF(ret);
            qDebug() << "av_parser_parse2 error" << errbuf;
            goto end;
        }

        // �����Ѿ�������������
        inData += ret;
        // ��ȥ�Ѿ�������������
        inLen -= ret;

        if (pkt->size <= 0) continue;
        if (decode(ctx, pkt, frame, outFile) < 0)
        {
            goto end;
        }

        // ����Ƿ���Ҫ��ȡ�µ��ļ�����
        if (inLen < REFILL_THRESH && !inEnd)
        {
            // ʣ�������ƶ�������������ǰ��
            memmove(inDataArray, inData, inLen);

            // ����inData
            inData = inDataArray;

            // ��ȡ�ļ����ݵ�inData + inLenλ��
            int len = inFile.read(inData + inLen, IN_DATA_SIZE - inLen);
            if (len > 0)    // ��ȡ���ļ�����
            {
                inLen += len;
            }
            else  // �ļ����Ѿ�û���κ�����
            {
                // ���Ϊ�Ѿ���ȡ���ļ���β��
                inEnd = 1;
            }
        }
    }

    // ˢ�»�����
    //      pkt->data = NULL;
    //      pkt->size = 0
    decode(ctx, nullptr, frame, outFile);

    // ��ֵ�������
    out.sampleRate = ctx->sample_rate;
    out.sampleFmt = ctx->sample_fmt;
    out.chLayout = ctx->channel_layout;
end:
    inFile.close();
    outFile.close();
    av_frame_free(&frame);
    av_packet_free(&pkt);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
}
