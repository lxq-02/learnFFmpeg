#include "xformat.h"
#include "xtools.h"
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

static int TimeoutCallback(void* para)
{
	auto xf = (XFormat*)para;
	if (xf->IsTimeout()) return 1;	// 超时退出Read
	//cout << "TimeOutCallback" << endl;


	return 0; // 正常阻塞
}

bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) // 清理原值
	{
		return false;
	}

	if (stream_index < 0 || stream_index > ctx_->nb_streams)
	{
		return false;
	}
	auto re = avcodec_parameters_copy(dst, ctx_->streams[stream_index]->codecpar);
	if (re < 0)
	{
		PrintErr(re);
		return false;
	}

	return true;
}

bool XFormat::CopyPara(int stream_index, AVCodecContext* dst)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) // 清理原值
	{
		return false;
	}

	if (stream_index < 0 || stream_index > ctx_->nb_streams)
	{
		return false;
	}
	auto re = avcodec_parameters_to_context(dst, ctx_->streams[stream_index]->codecpar);
	if (re < 0)
	{
		PrintErr(re);
		return false;
	}

	return true;
}

std::shared_ptr<XPara> XFormat::CopyVideoPara()
{
	int index = video_index();	// 拿到视频的index
	std::shared_ptr<XPara> re;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || !ctx_) return re;

	re.reset(XPara::Create());
	*re->time_base = ctx_->streams[index]->time_base; 
	avcodec_parameters_copy(re->para, ctx_->streams[index]->codecpar); // 复制视频参数
	return re;
}

std::shared_ptr<XPara> XFormat::CopyAudioPara()
{
	int index = audio_index();	// 拿到音频的index
	std::shared_ptr<XPara> re;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || !ctx_) return re;

	re.reset(XPara::Create());
	*re->time_base = ctx_->streams[index]->time_base;
	avcodec_parameters_copy(re->para, ctx_->streams[index]->codecpar); // 复制音频参数
	return re;
}

void XFormat::set_ctx(AVFormatContext* ctx)
{
	unique_lock<mutex> lock(mtx_);
	if (ctx_) // 清理原值
	{
		if (ctx_->oformat)  // 输出上下文
		{
			if (ctx_->pb) // 输出IO
			{
				avio_closep(&ctx_->pb); // 关闭输出IO
			}
			avformat_free_context(ctx_); // 释放输出上下文
		}
		else if (ctx_->iformat) // 输入上下文
		{
			avformat_close_input(&ctx_); // 关闭输入上下文
		}
		else
		{
			avformat_free_context(ctx_); // 释放上次的上下文
		}
	}
	ctx_ = ctx; // 设置新值
	if (!ctx_)
	{
		is_connected_ = false;
		return;
	}
	is_connected_ = true; // 设置连接状态为true

	// 计时，用于超时判断
	last_time_ = NowMs();

	// 设置超时处理回调
	if (time_out_ms_ > 0)
	{
		AVIOInterruptCB cb = { TimeoutCallback, this };
		ctx_->interrupt_callback = cb;
	}

	// 用于区分是否有音频或者视频流
	audio_index_ = -1;
	video_index_ = -1;


	// 区分音视频索引
	for (int i = 0; i < ctx->nb_streams; ++i)
	{
		// 音频
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index_ = i;
			audio_time_base_.den = ctx->streams[i]->time_base.den; // 音频流时间基准
			audio_time_base_.num = ctx->streams[i]->time_base.num; // 音频流时间基准
		}
		else if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index_ = i;
			video_time_base_.den = ctx->streams[i]->time_base.den; // 视频流时间基准
			video_time_base_.num = ctx->streams[i]->time_base.num; // 视频流时间基准
			video_codec_id_ = ctx->streams[i]->codecpar->codec_id; // 视频编解码器ID
		}
	}
}

bool XFormat::RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base)
{
	AVRational in_time_base = { time_base.num, time_base.den }; // 输入时间基准

	return RescaleTime(pkt, offset_pts, &in_time_base);
}

bool XFormat::RescaleTime(AVPacket* pkt, long long offset_pts, AVRational* time_base)
{
	if (!pkt || !time_base) return false;
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;

	auto out_stream = ctx_->streams[pkt->stream_index];

	// 重新计算pts dts duration
	// a * bq (输入basetime）/ cq（输出basetime） = a * bq / cq
	pkt->pts = av_rescale_q_rnd(pkt->pts - offset_pts, *time_base,
		out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->dts = av_rescale_q_rnd(pkt->dts - offset_pts, *time_base,
		out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->duration = av_rescale_q(pkt->duration, *time_base,
		out_stream->time_base);
	pkt->pos = -1; // 重置pos为-1，表示未知位置
	return true;
}

void XFormat::set_time_out_ms(int ms)
{
	unique_lock<mutex> lock(mtx_);
	this->time_out_ms_ = ms;

	// 设置回调函数，处理超时退出
	if (ctx_)
	{
		AVIOInterruptCB cb = { TimeoutCallback, this };
		ctx_->interrupt_callback = cb;
	}
}
