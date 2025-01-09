#pragma once

extern "C" {

    // �������API�����������
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

// ��Ƶ����
typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} AudioDecodeSpec;

class FFmpegs
{
public:
    FFmpegs();

    static void aacDecode(const char* inFilename,
        AudioDecodeSpec &out);
};

