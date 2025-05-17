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

FFmpegs::FFmpegs() {

}

// ������ظ�ʽ
static int check_pix_fmt(const AVCodec* codec,
    enum AVPixelFormat pixFmt) 
{
    const enum AVPixelFormat* p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE)
    {
        if (*p == pixFmt) return 1;
        p++;
    }
    return 0;
}

// ���ظ�������;�����˴���
// ����0����������������
static int encode(AVCodecContext* ctx,
    AVFrame* frame,
    AVPacket* pkt,
    QFile& outFile) 
{
    // �������ݵ�������
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // ���ϴӱ�������ȡ������������
    while (true)
    {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // ������ȡ���ݵ�frame��Ȼ���͵�������
            return 0;
        }
        else if (ret < 0) 
        { // ��������
            return ret;
        }

        // �ɹ��ӱ������õ�����������
        // ������������д���ļ�
        outFile.write((char*)pkt->data, pkt->size);

        // �ͷ�pkt�ڲ�����Դ
        av_packet_unref(pkt);
    }
}

void FFmpegs::h264Encode(VideoEncodeSpec& in, const char* outFilename) 
{
    // �ļ�
    QFile inFile(in.filename);
    QFile outFile(outFilename);

    // һ֡ͼƬ�Ĵ�С
    int imgSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1);

    // ���ؽ��
    int ret = 0;

    // ������
    AVCodec* codec = nullptr;

    // ����������
    AVCodecContext* ctx = nullptr;

    // ��ű���ǰ�����ݣ�yuv��
    AVFrame* frame = nullptr;

    // ��ű��������ݣ�h264��
    AVPacket* pkt = nullptr;

    //    uint8_t *buf = nullptr;

        // ��ȡ������
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) 
    {
        qDebug() << "encoder not found";
        return;
    }

    // ����������ݵĲ�����ʽ
    if (!check_pix_fmt(codec, in.pixFmt))
    {
        qDebug() << "unsupported pixel format"
            << av_get_pix_fmt_name(in.pixFmt);
        return;
    }

    // ��������������
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) 
    {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    // ����yuv����
    ctx->width = in.width;
    ctx->height = in.height;
    ctx->pix_fmt = in.pixFmt;
    // ����֡�ʣ�1������ʾ��֡����in.fps��
    ctx->time_base = { 1, in.fps };

    // �򿪱�����
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

    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    frame->pts = 0;

    // ����width��height��format����������
    ret = av_image_alloc(frame->data, frame->linesize,
        in.width, in.height, in.pixFmt, 1);
    if (ret < 0) 
    {
        ERROR_BUF(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    // �������뻺����������2��
//    buf = (uint8_t *) av_malloc(imgSize);
//    ret = av_image_fill_arrays(frame->data, frame->linesize,
//                               buf,
//                               in.pixFmt, in.width, in.height, 1);
//    if (ret < 0) {
//        ERROR_BUF(ret);
//        qDebug() << "av_image_fill_arrays error" << errbuf;
//        goto end;
//    }
//    qDebug() << buf << frame->data[0];

    // �������뻺����������3��
//    ret = av_frame_get_buffer(frame, 0);
//    if (ret < 0) {
//        ERROR_BUF(ret);
//        qDebug() << "av_frame_get_buffer error" << errbuf;
//        goto end;
//    }

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
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) 
    {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    // ��ȡ���ݵ�frame��
    while ((ret = inFile.read((char*)frame->data[0],
        imgSize)) > 0) 
    {
        // ���б���
        if (encode(ctx, frame, pkt, outFile) < 0)
        {
            goto end;
        }

        // ����֡�����
        frame->pts++;
    }

    // ˢ�»�����
    encode(ctx, nullptr, pkt, outFile);

end:
    // �ر��ļ�
    inFile.close();
    outFile.close();

    //    av_freep(&buf);

    // �ͷ���Դ
    if (frame)
    {
        av_freep(&frame->data[0]);
        //        av_free(frame->data[0]);
        //        frame->data[0] = nullptr;
        av_frame_free(&frame);
    }
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    qDebug() << "�߳���������";
}