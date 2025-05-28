#include "xdecode.h"
#include <iostream>
using namespace std;

extern "C" // 指令函数是C语言函数，函数名不包含重载标注
{
    // 引用ffmpeg头文件
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}


bool XDecode::Send(const AVPacket* pkt)
{
	unique_lock<std::mutex> lock(_mtx);
	if (!_ctx) return false;
    auto re = avcodec_send_packet(_ctx, pkt);
    if (re != 0) return false;
    return true;
}

bool XDecode::Recv(AVFrame* frame)
{
    unique_lock<std::mutex> lock(_mtx);
    if (!_ctx) return false;
    auto f = frame;
    if (_ctx->hw_device_ctx) // 硬件加速
    {
        f = av_frame_alloc();
    }
    auto re = avcodec_receive_frame(_ctx, f);
    if (re == 0)
    {
        if (_ctx->hw_device_ctx) // GPU解码
        {
            // 显存转内存 GPU=》CPU
			re = av_hwframe_transfer_data(frame, f, 0); // 转换数据到硬件帧
            av_frame_free(&f);
            if (re != 0)
            {
                PrintErr(re);
                return false;
            }
        }
        return true;
    }
    if (_ctx->hw_device_ctx) // 硬件加速
        av_frame_free(&f);
    return false;
}

std::vector<AVFrame*> XDecode::End()
{
    std::vector<AVFrame*> res;
    unique_lock<mutex> lock(_mtx);
    if (!_ctx) return res;

    // 取出缓存数据
    int ret = avcodec_send_packet(_ctx, nullptr);
    while (ret >= 0)
    {
        auto frame = av_frame_alloc();
        ret = avcodec_receive_frame(_ctx, frame);
        if (ret < 0)
        {
            av_frame_free(&frame);
            break;
        }
		res.push_back(frame); // 收集所有的AVFrame
        cout << frame->format << "-" << flush;
    }
    return res;
}

bool XDecode::InitHW(int type)
{
    unique_lock<mutex> lock(_mtx);
    if (!_ctx) return false;
    AVHWDeviceType;
    AVBufferRef* hw_device_ctx = nullptr; // 硬件加速上下文
	auto re = av_hwdevice_ctx_create(&hw_device_ctx, (AVHWDeviceType)type, nullptr, nullptr, 0);
    if (re != 0)
    {
        PrintErr(re);
        return false;
    }
	_ctx->hw_device_ctx = hw_device_ctx; // 设置硬件加速上下文
    cout << "硬件加速" << type << endl;

    return true;
}
