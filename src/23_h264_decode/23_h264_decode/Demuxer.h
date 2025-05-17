#ifndef DEMUXER_H
#define DEMUXER_H

#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} AudioDecodeSpec;

typedef struct {
    const char* filename;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;

class Demuxer {
public:
    Demuxer();

    void demux(const char* inFilename,
        AudioDecodeSpec& aOut,
        VideoDecodeSpec& vOut);

private:
    // ���װ������
    AVFormatContext* _fmtCtx = nullptr;
    // ����������
    AVCodecContext* _aDecodeCtx = nullptr, * _vDecodeCtx = nullptr;
    // ��
//    AVStream *_aStream = nullptr, *_vStream = nullptr;
    // ������
    int _aStreamIdx = 0, _vStreamIdx = 0;
    // �ļ�
    QFile _aOutFile, _vOutFile;
    // ��������
    AudioDecodeSpec* _aOut = nullptr;
    VideoDecodeSpec* _vOut = nullptr;
    // ��Ž���ǰ������
//    AVPacket *_pkt = nullptr;
    // ��Ž���������
    AVFrame* _frame = nullptr;
    // ���һ֡����ͼƬ�Ļ�����
    uint8_t* _imgBuf[4] = { nullptr };
    int _imgLinesizes[4] = { 0 };
    int _imgSize = 0;
    // ÿ����Ƶ����֡�����������������Ĵ�С
    int _sampleFrameSize = 0;
    // ÿһ����Ƶ�����Ĵ�С����������
    int _sampleSize = 0;

    int initVideoInfo();
    int initAudioInfo();
    int initDecoder(AVCodecContext** decodeCtx,
        int* streamIdx,
        AVMediaType type);
    int decode(AVCodecContext* decodeCtx,
        AVPacket* pkt,
        void (Demuxer::* func)());
    void writeVideoFrame();
    void writeAudioFrame();
};

#endif // DEMUXER_H