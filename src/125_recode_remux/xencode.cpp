#include "xencode.h"
#include <iostream>
using namespace std;

extern "C" // 指令函数是C语言函数，函数名不包含重载标注
{
	// 引用ffmpeg头文件
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

AVPacket* XEncode::Encode(const AVFrame* frame)
{
	if (!frame) return nullptr;
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return nullptr;

	// 发送到编码线程
	auto re = avcodec_send_frame(_ctx, frame);
	if (re != 0) return nullptr;
	auto pkt = av_packet_alloc();
	// 接收编码线程数据
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
	auto re = avcodec_send_frame(_ctx, nullptr); // 发送空帧，获取缓冲
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
		res.push_back(pkt); // 收集所有的AVPacket
	}
	return res;

}
