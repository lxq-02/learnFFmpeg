#include "demuxer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

#define END(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        goto end; \
    }

#define RET(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        return ret; \
    }

Demuxer::Demuxer() {

}

void Demuxer::demux(const char* inFilename,
    AudioDecodeSpec& aOut,
    VideoDecodeSpec& vOut) {
    // ��������
    _aOut = &aOut;
    _vOut = &vOut;

    AVPacket* pkt = nullptr;

    // ���ؽ��
    int ret = 0;

    // �������װ�����ġ����ļ�
    ret = avformat_open_input(&_fmtCtx, inFilename, nullptr, nullptr);
    END(avformat_open_input);

    // ��������Ϣ
    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    END(avformat_find_stream_info);

    // ��ӡ����Ϣ������̨
    av_dump_format(_fmtCtx, 0, inFilename, 0);
    fflush(stderr);

    // ��ʼ����Ƶ��Ϣ
    ret = initAudioInfo();
    if (ret < 0) {
        goto end;
    }

    // ��ʼ����Ƶ��Ϣ
    ret = initVideoInfo();
    if (ret < 0) {
        goto end;
    }

    // ��ʼ��frame
    _frame = av_frame_alloc();
    if (!_frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // ��ʼ��pkt
    pkt = av_packet_alloc();
    pkt->data = nullptr;
    pkt->size = 0;

    // �������ļ��ж�ȡ����
    while (av_read_frame(_fmtCtx, pkt) == 0) {
        if (pkt->stream_index == _aStreamIdx) { // ��ȡ��������Ƶ����
            ret = decode(_aDecodeCtx, pkt, &Demuxer::writeAudioFrame);
        }
        else if (pkt->stream_index == _vStreamIdx) { // ��ȡ��������Ƶ����
            ret = decode(_vDecodeCtx, pkt, &Demuxer::writeVideoFrame);
        }
        // �ͷ�pkt�ڲ�ָ��ָ���һЩ�����ڴ�
        av_packet_unref(pkt);

        if (ret < 0) {
            goto end;
        }
    }
    /*
    �෽������̬������������
    ���󷽷�����̬������������
    */

    // ˢ�»�����
//    AVPacket *pkt = _pkt;
//    _pkt = nullptr;
    decode(_aDecodeCtx, nullptr, &Demuxer::writeAudioFrame);
    decode(_vDecodeCtx, nullptr, &Demuxer::writeVideoFrame);

end:
    _aOutFile.close();
    _vOutFile.close();
    avcodec_free_context(&_aDecodeCtx);
    avcodec_free_context(&_vDecodeCtx);
    avformat_close_input(&_fmtCtx);
    av_frame_free(&_frame);
    av_packet_free(&pkt);
    av_freep(&_imgBuf[0]);
}

// ��ʼ����Ƶ��Ϣ
int Demuxer::initAudioInfo() {
    // ��ʼ��������
    int ret = initDecoder(&_aDecodeCtx, &_aStreamIdx, AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    // ���ļ�
    _aOutFile.setFileName(_aOut->filename);
    if (!_aOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _aOut->filename;
        return -1;
    }

    // ������Ƶ����
    _aOut->sampleRate = _aDecodeCtx->sample_rate;
    _aOut->sampleFmt = _aDecodeCtx->sample_fmt;
    _aOut->chLayout = _aDecodeCtx->channel_layout;

    // ��Ƶ����֡�Ĵ�С
    _sampleSize = av_get_bytes_per_sample(_aOut->sampleFmt);
    _sampleFrameSize = _sampleSize * _aDecodeCtx->channels;

    return 0;
}

// ��ʼ����Ƶ��Ϣ
int Demuxer::initVideoInfo() {
    // ��ʼ��������
    int ret = initDecoder(&_vDecodeCtx, &_vStreamIdx, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    // ���ļ�
    _vOutFile.setFileName(_vOut->filename);
    if (!_vOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _vOut->filename;
        return -1;
    }

    // ������Ƶ����
    _vOut->width = _vDecodeCtx->width;
    _vOut->height = _vDecodeCtx->height;
    _vOut->pixFmt = _vDecodeCtx->pix_fmt;
    // ֡��
    AVRational framerate = av_guess_frame_rate(
        _fmtCtx,
        _fmtCtx->streams[_vStreamIdx],
        nullptr);
    _vOut->fps = framerate.num / framerate.den;

    // �������ڴ��һ֡����ͼƬ�Ļ�����
    ret = av_image_alloc(_imgBuf, _imgLinesizes,
        _vOut->width, _vOut->height,
        _vOut->pixFmt, 1);
    RET(av_image_alloc);
    _imgSize = ret;

    return 0;
}

// ��ʼ��������
int Demuxer::initDecoder(AVCodecContext** decodeCtx,
    int* streamIdx,
    AVMediaType type) {
    // ����typeѰ������ʵ�����Ϣ
    // ����ֵ��������
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    RET(av_find_best_stream);

    // ������
    *streamIdx = ret;
    AVStream* stream = _fmtCtx->streams[*streamIdx];
    if (!stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    // Ϊ��ǰ���ҵ����ʵĽ�����
//    AVCodec *decoder = nullptr;
//    if (stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
//        decoder = avcodec_find_decoder_by_name("libfdk_aac");
//    } else {
//        decoder = avcodec_find_decoder(stream->codecpar->codec_id);
//    }
    AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found" << stream->codecpar->codec_id;
        return -1;
    }

    // ��ʼ������������
    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    // �����п���������������������
    ret = avcodec_parameters_to_context(*decodeCtx, stream->codecpar);
    RET(avcodec_parameters_to_context);

    // �򿪽�����
    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}

int Demuxer::decode(AVCodecContext* decodeCtx,
    AVPacket* pkt,
    void (Demuxer::* func)()) {
    // ����ѹ�����ݵ�������
    int ret = avcodec_send_packet(decodeCtx, pkt);
    RET(avcodec_send_packet);

    while (true) {
        // ��ȡ����������
        ret = avcodec_receive_frame(decodeCtx, _frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        RET(avcodec_receive_frame);

        // ִ��д���ļ��Ĵ���
        (this->*func)();

        //        // ��frame������д���ļ�
        //        if (decodeCtx->codec->type == AVMEDIA_TYPE_VIDEO) {
        //            writeVideoFrame();
        //        } else {
        //            writeAudioFrame();
        //        }
    }
}

void Demuxer::writeVideoFrame() {
    //    // д��Yƽ��
    //    _vOutFile.write((char *) _frame->data[0],
    //                    _frame->linesize[0] * _vOut->height);
    //    // д��Uƽ��
    //    _vOutFile.write((char *) _frame->data[1],
    //                    _frame->linesize[1] * _vOut->height >> 1);
    //    // д��Vƽ��
    //    _vOutFile.write((char *) _frame->data[2],
    //                    _frame->linesize[2] * _vOut->height >> 1);

        // ����frame�����ݵ�_imgBuf������
    av_image_copy(_imgBuf, _imgLinesizes,
        (const uint8_t**)(_frame->data), _frame->linesize,
        _vOut->pixFmt, _vOut->width, _vOut->height);
    // ��������������д���ļ�
    _vOutFile.write((char*)_imgBuf[0], _imgSize);
}

void Demuxer::writeAudioFrame() {
    // libfdk_aac�����������������PCM��ʽ��s16
    // aac�����������������PCM��ʽ��ftlp

    // LLLL RRRR DDDD FFFF

    if (av_sample_fmt_is_planar(_aOut->sampleFmt)) { // planar
        // ���ѭ����ÿһ��������������
        // si = sample index
        for (int si = 0; si < _frame->nb_samples; si++) {
            // �ڲ�ѭ�����ж��ٸ�����
            // ci = channel index
            for (int ci = 0; ci < _aDecodeCtx->channels; ci++) {
                char* begin = (char*)(_frame->data[ci] + si * _sampleSize);
                _aOutFile.write(begin, _sampleSize);
            }
        }
    }
    else { // ��planar
        //        _aOutFile.write((char *) _frame->data[0], _frame->linesize[0]);
        _aOutFile.write((char*)_frame->data[0],
            _frame->nb_samples * _sampleFrameSize);
    }
}