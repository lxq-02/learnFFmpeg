#include "xmux.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}
#define BERR(err) if (err != 0) { PrintErr(err); return 0; } 

AVFormatContext* XMux::Open(const char* url,
	AVCodecParameters* video_para,
	AVCodecParameters* audio_para)
{
	AVFormatContext* ctx = nullptr;
	// 创建上下文
	auto re = avformat_alloc_output_context2(&ctx, NULL, NULL,
		url // 根据文件名推测封装格式
	);
	BERR(re);

	// 添加视频流、音频流
	if (video_para)
	{
		auto vs = avformat_new_stream(ctx, NULL);	// 视频流
		avcodec_parameters_copy(vs->codecpar, video_para);	// 复制视频参数
	}
	if (audio_para)
	{
		auto as = avformat_new_stream(ctx, NULL);	// 音频流
		avcodec_parameters_copy(as->codecpar, audio_para);	// 复制音频参数
	}

	// 打开输出IO
	re = avio_open(&ctx->pb, url, AVIO_FLAG_WRITE);
	BERR(re);
	av_dump_format(ctx, 0, url, 1); // 打印封装信息
	return ctx;
}

bool XMux::WriteHead()
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = avformat_write_header(ctx_, NULL);
	BERR(re);
	// 打印封装信息
	av_dump_format(ctx_, 0, ctx_->url, 1); 
	return true;
}

bool XMux::Write(AVPacket* pkt)
{
	if (!pkt) return false;
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	// 没读取到pts 重构考虑通过duration计算
	if (pkt->pts == AV_NOPTS_VALUE)
	{
		pkt->pts = 0;
		pkt->dts = 0;
	}
	if (pkt->stream_index == video_index_)
	{
		if (begin_video_pts_ < 0)
		{
			begin_video_pts_ = pkt->pts;	// 记录视频开始时间
		}
		lock.unlock();
		RescaleTime(pkt, begin_video_pts_, src_video_time_base_);
		lock.lock();
	}
	else if (pkt->stream_index == audio_index_)
	{
		if (begin_audio_pts_ < 0)
		{
			begin_audio_pts_ = pkt->pts;	// 记录音频开始时间
		}
		lock.unlock();
		RescaleTime(pkt, begin_audio_pts_, src_audio_time_base_);
		lock.lock();
	}

	cout << pkt->pts << " " << flush;
	// 写入一帧数据，内部缓冲排序dts，通过pkt=null 可以写入缓冲
	auto re = av_interleaved_write_frame(ctx_, pkt);
	BERR(re);
	return true;
}

bool XMux::WriteEnd()
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	av_interleaved_write_frame(ctx_, nullptr);	// 写入排序缓冲
	auto re = av_write_trailer(ctx_);
	BERR(re);

	return true;
}

void XMux::set_src_video_time_base(AVRational* tb)
{
	if (!tb) return;	// 空指针不处理
	unique_lock<mutex> lock(mtx_);
	if (!src_video_time_base_)
	{
		src_video_time_base_ = new AVRational();
	}
	*src_video_time_base_ = *tb;
}

void XMux::set_src_audio_time_base(AVRational* tb)
{
	if (!tb) return;	// 空指针不处理
	unique_lock<mutex> lock(mtx_);
	if (!src_audio_time_base_)
	{
		src_audio_time_base_ = new AVRational();
	}
	*src_audio_time_base_ = *tb;
}

XMux::~XMux()
{
	unique_lock<mutex> lock(mtx_);
	if (!src_video_time_base_)
	{
		delete src_video_time_base_;
	}
	src_video_time_base_ = nullptr;
	if (!src_audio_time_base_)
	{
		delete src_audio_time_base_;
	}
	src_audio_time_base_ = nullptr;
}
