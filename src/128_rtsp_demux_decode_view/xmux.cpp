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
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = avformat_write_header(ctx_, NULL);
	BERR(re);
	// ��ӡ��װ��Ϣ
	av_dump_format(ctx_, 0, ctx_->url, 1); 
	return true;
}

bool XMux::Write(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	// д��һ֡���ݣ��ڲ���������dts��ͨ��pkt=null ����д�뻺��
	auto re = av_interleaved_write_frame(ctx_, pkt);
	BERR(re);
	return true;
}

bool XMux::WriteEnd()
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	av_interleaved_write_frame(ctx_, nullptr);	// д�����򻺳�
	auto re = av_write_trailer(ctx_);
	BERR(re);

	return true;
}
