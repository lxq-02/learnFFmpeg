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
	if (xf->IsTimeout()) return 1;	// ��ʱ�˳�Read
	//cout << "TimeOutCallback" << endl;


	return 0; // ��������
}

bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) // ����ԭֵ
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
	if (!ctx_) // ����ԭֵ
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
	int index = video_index();	// �õ���Ƶ��index
	std::shared_ptr<XPara> re;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || !ctx_) return re;

	re.reset(XPara::Create());
	*re->time_base = ctx_->streams[index]->time_base; 
	avcodec_parameters_copy(re->para, ctx_->streams[index]->codecpar); // ������Ƶ����
	return re;
}

std::shared_ptr<XPara> XFormat::CopyAudioPara()
{
	int index = audio_index();	// �õ���Ƶ��index
	std::shared_ptr<XPara> re;
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || !ctx_) return re;

	re.reset(XPara::Create());
	*re->time_base = ctx_->streams[index]->time_base;
	avcodec_parameters_copy(re->para, ctx_->streams[index]->codecpar); // ������Ƶ����
	return re;
}

void XFormat::set_ctx(AVFormatContext* ctx)
{
	unique_lock<mutex> lock(mtx_);
	if (ctx_) // ����ԭֵ
	{
		if (ctx_->oformat)  // ���������
		{
			if (ctx_->pb) // ���IO
			{
				avio_closep(&ctx_->pb); // �ر����IO
			}
			avformat_free_context(ctx_); // �ͷ����������
		}
		else if (ctx_->iformat) // ����������
		{
			avformat_close_input(&ctx_); // �ر�����������
		}
		else
		{
			avformat_free_context(ctx_); // �ͷ��ϴε�������
		}
	}
	ctx_ = ctx; // ������ֵ
	if (!ctx_)
	{
		is_connected_ = false;
		return;
	}
	is_connected_ = true; // ��������״̬Ϊtrue

	// ��ʱ�����ڳ�ʱ�ж�
	last_time_ = NowMs();

	// ���ó�ʱ����ص�
	if (time_out_ms_ > 0)
	{
		AVIOInterruptCB cb = { TimeoutCallback, this };
		ctx_->interrupt_callback = cb;
	}

	// ���������Ƿ�����Ƶ������Ƶ��
	audio_index_ = -1;
	video_index_ = -1;


	// ��������Ƶ����
	for (int i = 0; i < ctx->nb_streams; ++i)
	{
		// ��Ƶ
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index_ = i;
			audio_time_base_.den = ctx->streams[i]->time_base.den; // ��Ƶ��ʱ���׼
			audio_time_base_.num = ctx->streams[i]->time_base.num; // ��Ƶ��ʱ���׼
		}
		else if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index_ = i;
			video_time_base_.den = ctx->streams[i]->time_base.den; // ��Ƶ��ʱ���׼
			video_time_base_.num = ctx->streams[i]->time_base.num; // ��Ƶ��ʱ���׼
			video_codec_id_ = ctx->streams[i]->codecpar->codec_id; // ��Ƶ�������ID
		}
	}
}

bool XFormat::RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base)
{
	AVRational in_time_base = { time_base.num, time_base.den }; // ����ʱ���׼

	return RescaleTime(pkt, offset_pts, &in_time_base);
}

bool XFormat::RescaleTime(AVPacket* pkt, long long offset_pts, AVRational* time_base)
{
	if (!pkt || !time_base) return false;
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;

	auto out_stream = ctx_->streams[pkt->stream_index];

	// ���¼���pts dts duration
	// a * bq (����basetime��/ cq�����basetime�� = a * bq / cq
	pkt->pts = av_rescale_q_rnd(pkt->pts - offset_pts, *time_base,
		out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->dts = av_rescale_q_rnd(pkt->dts - offset_pts, *time_base,
		out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->duration = av_rescale_q(pkt->duration, *time_base,
		out_stream->time_base);
	pkt->pos = -1; // ����posΪ-1����ʾδ֪λ��
	return true;
}

void XFormat::set_time_out_ms(int ms)
{
	unique_lock<mutex> lock(mtx_);
	this->time_out_ms_ = ms;

	// ���ûص�����������ʱ�˳�
	if (ctx_)
	{
		AVIOInterruptCB cb = { TimeoutCallback, this };
		ctx_->interrupt_callback = cb;
	}
}
