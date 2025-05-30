#include "xcodec.h"
#include <iostream>
using namespace std;

extern "C" // 指令函数是C语言函数，函数名不包含重载标注
{
	// 引用ffmpeg头文件
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

// 预处理指令导入库
#pragma comment(lib, "avcodec.lib")

void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

AVCodecContext* XCodec::Create(int codec_id, bool is_encode)
{
	// 1、找到编码器
	AVCodec* codec = nullptr;
	if (is_encode)
		codec = avcodec_find_encoder((AVCodecID)codec_id);
	else
		codec = avcodec_find_decoder((AVCodecID)codec_id);
	if (!codec)
	{
		cerr << "avcodec_find_encoder failed!" << codec_id << endl;
		return nullptr;
	}

	// 2、编码上下文，设定上下文参数
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cerr << "avcodec_alloc_context3 failed!" << codec_id << endl;
		return nullptr;
	}

	// 设置参数默认值
	ctx->time_base = { 1, 25 }; // 分数 1/25
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	ctx->thread_count = 16;
	return ctx;
}

void XCodec::set_context(AVCodecContext* ctx)
{
	unique_lock<std::mutex> lock(mtx_);
	if (ctx_)
	{
		avcodec_free_context(&ctx_); // 释放之前的上下文
	}
	this->ctx_ = ctx;
}

bool XCodec::SetOpt(const char* key, const char* val)
{
	unique_lock<std::mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_opt_set(ctx_->priv_data, key, val, 0);
	if (re != 0)
	{
		cerr << "av_opt_set failed!";
		PrintErr(re);
	}
	return true;
}

bool XCodec::SetOpt(const char* key, int val)
{
	unique_lock<std::mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_opt_set_int(ctx_->priv_data, key, val, 0);
	if (re != 0)
	{
		cerr << "av_opt_set failed!";
		PrintErr(re);
	}
	return true;
}

bool XCodec::Open()
{
	unique_lock<std::mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = avcodec_open2(ctx_, NULL, NULL);
	if (re != 0)
	{
		PrintErr(re);
		return false;
	}
	return true;
}

AVFrame* XCodec::CreateFrame()
{
	unique_lock<std::mutex> lock(mtx_);
	if (!ctx_) return nullptr;
	auto frame = av_frame_alloc();
	frame->width = ctx_->width;
	frame->height = ctx_->height;
	frame->format = ctx_->pix_fmt;
	auto re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		av_frame_free(&frame);
		PrintErr(re);
		return nullptr;
	}
	return frame;
}
