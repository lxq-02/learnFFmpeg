#pragma once

extern "C" {

    // 工具相关API（比如错误处理）
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

// 音频参数
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

