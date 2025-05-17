#pragma once

extern "C" {

    // �������API�����������
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

// ��Ƶ����
typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} ResampleAudioSpec;

class FFmpegs
{
public:
    static void resampleAudio(ResampleAudioSpec& in, ResampleAudioSpec& out);

    static void resampleAudio(const char* inFilename,
                              int inSampleRate,
                              AVSampleFormat inSampleFmt,
                              int inChLayout,

                              const char* outFilename,
                              int outSampleRate,
                              AVSampleFormat outSampleFmt,
                              int outChLayout);
};

