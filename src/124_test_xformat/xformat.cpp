#include "xformat.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) // 清理原值
	{
		return false;
	}

	if (stream_index < 0 || stream_index > _ctx->nb_streams)
	{
		return false;
	}
	auto re = avcodec_parameters_copy(dst, _ctx->streams[stream_index]->codecpar);
	if (re < 0)
	{
		PrintErr(re);
		return false;
	}

	return true;
}

void XFormat::set_ctx(AVFormatContext* ctx)
{
	unique_lock<mutex> lock(_mtx);
	if (_ctx) // 清理原值
	{
		if (_ctx->oformat)  // 输出上下文
		{
			if (_ctx->pb) // 输出IO
			{
				avio_closep(&_ctx->pb); // 关闭输出IO
			}
			avformat_free_context(_ctx); // 释放输出上下文
		}
		else if (_ctx->iformat) // 输入上下文
		{
			avformat_close_input(&_ctx); // 关闭输入上下文
		}
		else
		{
			avformat_free_context(_ctx); // 释放上次的上下文
		}
	}
	_ctx = ctx; // 设置新值
	if (!_ctx) return;


	// 区分音视频索引
	for (int i = 0; i < ctx->nb_streams; ++i)
	{
		// 音频
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			_audio_index = i;
		}
		else if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			_video_index = i;
		}
	}
}
