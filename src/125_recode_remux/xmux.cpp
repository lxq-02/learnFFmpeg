#include "xmux.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}
#define BERR(err) if (err != 0) { PrintErr(err); return 0; } 

AVFormatContext* XMux::Open(const char* url)
{
	AVFormatContext* ctx = nullptr;
	// 创建上下文
	auto re = avformat_alloc_output_context2(&ctx, NULL, NULL,
		url // 根据文件名推测封装格式
	);
	BERR(re);

	// 添加视频流、音频流
	auto vs = avformat_new_stream(ctx, NULL);	// 视频流
	vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;	// 设置视频流类型
	auto as = avformat_new_stream(ctx, NULL);	// 音频流
	as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;	// 设置音频流类型

	// 打开输出IO
	re = avio_open(&ctx->pb, url, AVIO_FLAG_WRITE);
	BERR(re);
	return ctx;
}

bool XMux::WriteHead()
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = avformat_write_header(_ctx, NULL);
	BERR(re);
	// 打印封装信息
	av_dump_format(_ctx, 0, _ctx->url, 1); 
	return true;
}

bool XMux::Write(AVPacket* pkt)
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	// 写入一帧数据，内部缓冲排序dts，通过pkt=null 可以写入缓冲
	auto re = av_interleaved_write_frame(_ctx, pkt);
	BERR(re);
	return true;
}

bool XMux::WriteEnd()
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	av_interleaved_write_frame(_ctx, nullptr);	// 写入排序缓冲
	auto re = av_write_trailer(_ctx);
	BERR(re);

	return true;
}
