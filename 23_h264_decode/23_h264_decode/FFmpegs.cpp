#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

// ���뻺�����Ĵ�С
#define IN_DATA_SIZE 4096

FFmpegs::FFmpegs() {

}

static int frameIdx = 0;

static int decode(AVCodecContext* ctx,
    AVPacket* pkt,
    AVFrame* frame,
    QFile& outFile) {
    // ����ѹ�����ݵ�������
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    while (true) {
        // ��ȡ����������
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        else if (ret < 0) {
            ERROR_BUF(ret);
            qDebug() << "avcodec_receive_frame error" << errbuf;
            return ret;
        }

        qDebug() << "�������" << ++frameIdx << "֡";

        // ������������д���ļ�
        // д��Yƽ��
        outFile.write((char*)frame->data[0],
            frame->linesize[0] * ctx->height);
        // д��Uƽ��
        outFile.write((char*)frame->data[1],
            frame->linesize[1] * ctx->height >> 1);
        // д��Vƽ��
        outFile.write((char*)frame->data[2],
            frame->linesize[2] * ctx->height >> 1);

        //        qDebug() << frame->data[0] << frame->data[1] << frame->data[2];

                /*
                 * frame->data[0] 0xd08c400 0x8c400
                 * frame->data[1] 0xd0d79c0 0xd79c0
                 * frame->data[2] 0xd0ea780 0xea780
                 *
                 * frame->data[1] - frame->data[0] = 308672 = yƽ��Ĵ�С
                 * frame->data[2] - frame->data[1] = 77248 = uƽ��Ĵ�С
                 *
                 * yƽ��Ĵ�С 640x480*1 = 307200
                 * uƽ��Ĵ�С 640x480*0.25 = 76800
                 * vƽ��Ĵ�С 640x480*0.25
                 */

                 //        // ������������д���ļ�(460800)
                 //        int imgSize = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 1);
                 //        // outFile.write((char *) frame->data[0], frame->linesize[0]);
                 //        outFile.write((char *) frame->data[0], imgSize);
    }
}

void FFmpegs::h264Decode(const char* inFilename,
                        VideoDecodeSpec& out)
{
    // ���ؽ��
    int ret = 0;

    // ������Ŷ�ȡ�������ļ����ݣ�h264��
    // ����AV_INPUT_BUFFER_PADDING_SIZE��Ϊ�˷�ֹĳЩ�Ż�����readerһ���Զ�ȡ���ർ��Խ��
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char* inData = inDataArray;

    // ÿ�δ������ļ��ж�ȡ�ĳ��ȣ�h264��
    // ���뻺�����У�ʣ�µĵȴ����н������Ч���ݳ���
    int inLen;
    // �Ƿ��Ѿ���ȡ���������ļ���β��
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

    // ��Ž���ǰ������(h264)
    AVPacket* pkt = nullptr;
    // ��Ž���������(yuv)
    AVFrame* frame = nullptr;

    // ��ȡ������
    //    codec = avcodec_find_decoder_by_name("h264");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
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
        qDebug() << "avcodec_open2 error" << errbuf;
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
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    // ��ȡ�ļ�����
    do 
    {
        inLen = inFile.read(inDataArray, IN_DATA_SIZE);
        // �����Ƿ����ļ�β��
        inEnd = !inLen;

        // ��inDataָ���������Ԫ��
        inData = inDataArray;

        // ֻҪ���뻺�����л��еȴ����н��������
        while (inLen > 0 || inEnd) 
        {
            // �����ļ�β������Ȼû�ж�ȡ�κ����ݣ���ҲҪ����av_parser_parse2���޸�bug��

            // ��������������
            ret = av_parser_parse2(parserCtx, ctx,
                &pkt->data, &pkt->size,
                (uint8_t*)inData, inLen,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

            if (ret < 0) 
            {
                ERROR_BUF(ret);
                qDebug() << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            // �����Ѿ�������������
            inData += ret;
            // ��ȥ�Ѿ������������ݴ�С
            inLen -= ret;

            qDebug() << inEnd << pkt->size << ret;

            // ����
            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) 
            {
                goto end;
            }

            // ��������ļ�β��
            if (inEnd) break;
        }
    } while (!inEnd);

    // ˢ�»�����
    //    pkt->data = nullptr;
    //    pkt->size = 0;
    //    decode(ctx, pkt, frame, outFile);
    decode(ctx, nullptr, frame, outFile);

    // ��ֵ�������
    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    // ��framerate.num��ȡ֡�ʣ�������time_base.den
    out.fps = ctx->framerate.num;

end:
    inFile.close();
    outFile.close();
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);

    // bug fix
    // https://patchwork.ffmpeg.org/project/ffmpeg/patch/tencent_609A2E9F73AB634ED670392DD89A63400008@qq.com/

    //
    //    while ((inLen = inFile.read(inDataArray, IN_DATA_SIZE)) > 0)
    //        while (inLen > 0) {
    //            // ��������������
    //            ret = av_parser_parse2(parserCtx, ctx,
    //                                   &pkt->data, &pkt->size,
    //                                   (uint8_t *) inData, inLen,
    //                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

    //            if (ret < 0) {
    //                ERROR_BUF(ret);
    //                qDebug() << "av_parser_parse2 error" << errbuf;
    //                goto end;
    //            }

    //            // �����Ѿ�������������
    //            inData += ret;
    //            // ��ȥ�Ѿ������������ݴ�С
    //            inLen -= ret;

    //            // ����
    //            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
    //                goto end;
    //            }
    //        }
    //    }
}