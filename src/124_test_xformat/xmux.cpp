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
	// ����������
	auto re = avformat_alloc_output_context2(&ctx, NULL, NULL,
		url // �����ļ����Ʋ��װ��ʽ
	);
	BERR(re);

	// �����Ƶ������Ƶ��
	auto vs = avformat_new_stream(ctx, NULL);	// ��Ƶ��
	vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;	// ������Ƶ������
	auto as = avformat_new_stream(ctx, NULL);	// ��Ƶ��
	as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;	// ������Ƶ������

	// �����IO
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
	// ��ӡ��װ��Ϣ
	av_dump_format(_ctx, 0, _ctx->url, 1); 
	return true;
}

bool XMux::Write(AVPacket* pkt)
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	// д��һ֡���ݣ��ڲ���������dts��ͨ��pkt=null ����д�뻺��
	auto re = av_interleaved_write_frame(_ctx, pkt);
	BERR(re);
	return true;
}

bool XMux::WriteEnd()
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	av_interleaved_write_frame(_ctx, nullptr);	// д�����򻺳�
	auto re = av_write_trailer(_ctx);
	BERR(re);

	return true;
}
