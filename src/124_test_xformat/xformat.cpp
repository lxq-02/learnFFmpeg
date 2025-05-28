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
	if (!_ctx) // ����ԭֵ
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
	if (_ctx) // ����ԭֵ
	{
		if (_ctx->oformat)  // ���������
		{
			if (_ctx->pb) // ���IO
			{
				avio_closep(&_ctx->pb); // �ر����IO
			}
			avformat_free_context(_ctx); // �ͷ����������
		}
		else if (_ctx->iformat) // ����������
		{
			avformat_close_input(&_ctx); // �ر�����������
		}
		else
		{
			avformat_free_context(_ctx); // �ͷ��ϴε�������
		}
	}
	_ctx = ctx; // ������ֵ
	if (!_ctx) return;
	// ���������Ƿ�����Ƶ������Ƶ��
	_audio_index = -1;
	_video_index = -1;


	// ��������Ƶ����
	for (int i = 0; i < ctx->nb_streams; ++i)
	{
		// ��Ƶ
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			_audio_index = i;
			_audio_time_base.den = ctx->streams[i]->time_base.den; // ��Ƶ��ʱ���׼
			_audio_time_base.num = ctx->streams[i]->time_base.num; // ��Ƶ��ʱ���׼
		}
		else if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			_video_index = i;
			_video_time_base.den = ctx->streams[i]->time_base.den; // ��Ƶ��ʱ���׼
			_video_time_base.num = ctx->streams[i]->time_base.num; // ��Ƶ��ʱ���׼
		}
	}
}
