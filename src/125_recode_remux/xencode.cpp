#include "xencode.h"
#include <iostream>
using namespace std;

extern "C" // ָ�����C���Ժ��������������������ر�ע
{
	// ����ffmpegͷ�ļ�
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
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
