#pragma once

extern "C"
{
#include <libavutil/avutil.h>
}

typedef struct
{
	char* pixels;
	int width;
	int height;
	AVPixelFormat format;
}RawVideoFrame;

typedef struct
{
	const char* filename;
	int width;
	int height;
	AVPixelFormat format;
}RawVidelFile;

class FFmpegs
{
public:
	FFmpegs();
	static void convertRawVideo(RawVideoFrame in, RawVideoFrame out);
	static void convertRawVideo(RawVidelFile in, RawVidelFile out);
};

