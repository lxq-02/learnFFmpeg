#pragma once

extern "C" 
{
#define __STDC_CONSTANT_MACROS

#include <libavutil/avutil.h>
}

typedef struct 
{
    const char* filename;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;

class FFmpegs {
public:
    FFmpegs();

    static void h264Decode(const char* inFilename,
        VideoDecodeSpec& out);
};

