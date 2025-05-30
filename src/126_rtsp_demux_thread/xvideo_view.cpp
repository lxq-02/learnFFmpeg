#include "xvideo_view.h"
#include "xsdl.h"
#include <chrono>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include "xtools.h"

#pragma comment(lib, "winmm.lib")

extern "C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avutil.lib")

using namespace std;


XVideoView* XVideoView::Create(RenderType type)
{
    switch (type)
    {
    case RenderType::SDL:
        return new XSDL();
    default:
        break;
    }
    return nullptr;
}

bool XVideoView::DrawFrame(AVFrame* frame)
{
    if (!frame || !frame->data[0]) return false;

    count_++;
    auto now = chrono::high_resolution_clock::now();
    if (beg_ms_.time_since_epoch().count() == 0)
    {
        // 说明还未初始化
        beg_ms_ = now;
    }
    else 
    {
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - beg_ms_).count();
        if (elapsed >= 1000)
        {
            render_fps_ = count_;
            count_ = 0;
            beg_ms_ = now;
        }
    }

    int linesize = 0;
    switch (frame->format)
    {
    case AV_PIX_FMT_YUV420P:
        return Draw(frame->data[0], frame->linesize[0], // Y
            frame->data[1], frame->linesize[1], // U
            frame->data[2], frame->linesize[2]  // V
        );
    case AV_PIX_FMT_NV12:
		if (!cache_)
		{
			// 分配缓存空间
			cache_ = new unsigned char[4096 * 2160 * 1.5];
		}
        linesize = frame->width;
        if (frame->linesize[0] == frame->width)
        {
            memcpy(cache_, frame->data[0], frame->linesize[0] * frame->height); // Y
            memcpy(cache_ + frame->linesize[0] * frame->height, frame->data[1], frame->linesize[1] * frame->height / 2); // UV
        }
        else // 逐行复制
        {
            for (int i = 0; i < frame->height; i++)
            {
				memcpy(cache_ + i * frame->width, frame->data[0] + i * frame->linesize[0], frame->width); // Y
            }
            for (int i = 0; i < frame->height / 2; i++) // UV
            {
                auto p = cache_ + frame->height * frame->width; // 移位Y
				memcpy(p + i * frame->width, frame->data[1] + i * frame->linesize[1], frame->width); 
            }
        }

        // frame->data[0] + frame->data[1]
        return Draw(cache_, frame->linesize[0]);
    case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_RGB24:
        return Draw(frame->data[0], frame->linesize[0]);
    defautl:
        break;
    }
    return false;
}

bool XVideoView::Open(std::string filepath)
{
    if (ifs_.is_open())
    {
        ifs_.close();
    }
    ifs_.open(filepath, ios::binary);
    return ifs_.is_open();
}

AVFrame* XVideoView::Read()
{
    if (width_ <= 0 || height_ <= 0 || !ifs_) return nullptr;
    // AVFrame空间已经申请，如果参数发生变化，需要释放空间
    if (frame_)
    {
        if (frame_->width != width_
            || frame_->height != height_
            || frame_->format != fmt_)
        {
            // 释放AVFrame对象空间，和buf引用计数减一
            av_frame_free(&frame_);
        }
    }
    if (!frame_)
    {
        // 分配对象空间和像素空间
        frame_ = av_frame_alloc();
        frame_->width = width_;
        frame_->height = height_;
        frame_->format = fmt_;
        frame_->linesize[0] = width_ * 4;
        if (frame_->format == AV_PIX_FMT_YUV420P)
        {
            frame_->linesize[0] = width_; // Y
            frame_->linesize[1] = width_ / 2; // U
            frame_->linesize[2] = width_ / 2; // V
        }
        else if (frame_->format == AV_PIX_FMT_RGB24)
        {
            frame_->linesize[0] = width_ * 3;
        }
        // 生成AVFrame空间，使用默认对齐
        auto re = av_frame_get_buffer(frame_, 0);
        if (re != 0)
        {
            char buf[1024] = { 0 };
            av_strerror(re, buf, sizeof(buf));
            cout << buf << endl;
            av_frame_free(&frame_);
            return nullptr;
        }
    }

    if (!frame_) return nullptr;

    // 读取一帧数据
    if (frame_->format == AV_PIX_FMT_YUV420P)
    {
        ifs_.read((char*)frame_->data[0], frame_->linesize[0] * frame_->height); // Y
        ifs_.read((char*)frame_->data[1], frame_->linesize[1] * frame_->height / 2); // U
        ifs_.read((char*)frame_->data[2], frame_->linesize[2] * frame_->height / 2); // V
    }
    else // RGBA ARGB BGRA 32 RGB24
    {
        ifs_.read((char*)frame_->data[0], frame_->linesize[0] * frame_->height);
    }
    if (ifs_.gcount() == 0)
        return nullptr;
    return frame_;
}

XVideoView::~XVideoView()
{
	if (frame_)
	{
		av_frame_free(&frame_);
		frame_ = nullptr;
	}
	if (cache_)
	{
		delete[] cache_;
		cache_ = nullptr;
	}
	if (ifs_.is_open())
	{
		ifs_.close();
	}
}
