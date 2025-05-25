#include "xencode.h"
#include <iostream>
using namespace std;

extern "C" // ָ�����C���Ժ��������������������ر�ע
{
	// ����ffmpegͷ�ļ�
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

// Ԥ����ָ����
#pragma comment(lib, "avcodec.lib")

static void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

AVCodecContext* XEncode::Create(int codec_id)
{
	// 1���ҵ�������
	AVCodec* codec = avcodec_find_encoder((AVCodecID)codec_id);
	if (!codec)
	{
		cerr << "avcodec_find_encoder failed!" << codec_id << endl;
		return nullptr;
	}

	// 2�����������ģ��趨�����Ĳ���
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cerr << "avcodec_alloc_context3 failed!" << codec_id << endl;
		return nullptr;
	}

	// ���ò���Ĭ��ֵ
	ctx->time_base = { 1, 25 }; // ���� 1/25
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	ctx->thread_count = 16;
	return ctx;
}

void XEncode::set_context(AVCodecContext* ctx)
{
	unique_lock<std::mutex> lock(_mtx);
	if (_ctx)
	{
		avcodec_free_context(&_ctx); // �ͷ�֮ǰ��������
	}
	this->_ctx = ctx;
}

bool XEncode::SetOpt(const char* key, const char* val)
{
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = av_opt_set(_ctx->priv_data, key, val, 0);
	if (re != 0)
	{
		cerr << "av_opt_set failed!";
		PrintErr(re);		
	}
	return true;
}

bool XEncode::SetOpt(const char* key, int val)
{
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = av_opt_set_int(_ctx->priv_data, key, val, 0);
	if (re != 0)
	{
		cerr << "av_opt_set failed!";
		PrintErr(re);
	}
	return true;
}

bool XEncode::Open()
{
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = avcodec_open2(_ctx, NULL, NULL); 
	if (re != 0)
	{
		PrintErr(re);
		return false;
	}
	return true;
}

AVPacket* XEncode::Encode(const AVFrame* frame)
{
	if (!frame) return nullptr;
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return nullptr;

	// ���͵������߳�
	auto re = avcodec_send_frame(_ctx, frame);
	if (re != 0) return nullptr;
	auto pkt = av_packet_alloc();
	// ���ձ����߳�����
	re = avcodec_receive_packet(_ctx, pkt);
	if (re == 0)
	{
		return pkt;
	}
	av_packet_free(&pkt);
	if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
	{
		return nullptr;
	}
	if (re < 0)
	{
		PrintErr(re);
		return nullptr;
	}

	return nullptr;
}

AVFrame* XEncode::CreateFrame()
{
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return nullptr;
	auto frame = av_frame_alloc();
	frame->width = _ctx->width;
	frame->height = _ctx->height;
	frame->format = _ctx->pix_fmt;
	auto re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		av_frame_free(&frame);
		PrintErr(re);
		return nullptr;
	}
	return frame;
}

std::vector<AVPacket*> XEncode::End()
{
	std::vector<AVPacket*> res;
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return res;
	auto re = avcodec_send_frame(_ctx, nullptr); // ���Ϳ�֡����ȡ����
	if (re != 0) return res;
	while (re >= 0)
	{
		auto pkt = av_packet_alloc();
		re = avcodec_receive_packet(_ctx, pkt);
		if (re != 0)
		{
			av_packet_free(&pkt);
			break;
		}
		res.push_back(pkt); // �ռ����е�AVPacket
	}
	return res;

}
