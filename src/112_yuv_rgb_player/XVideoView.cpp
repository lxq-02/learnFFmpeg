#include "XVideoView.h"
#include "XSDL.h"
#include <chrono>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>


#pragma comment(lib, "winmm.lib")

extern "C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avutil.lib")

using namespace std;

void MSleep(unsigned int ms)
{
    auto beg = chrono::high_resolution_clock::now();
    while (true)
    {
        auto now = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - beg).count();
        if (elapsed >= ms) break;

        // 剩余时间 >1ms 才 sleep
        if (ms - elapsed > 1)
        {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
}

long long NowMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}



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

    _count++;
    auto now = chrono::high_resolution_clock::now();
    if (_beg_ms.time_since_epoch().count() == 0)
    {
        // 说明还未初始化
        _beg_ms = now;
    }
    else 
    {
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - _beg_ms).count();
        if (elapsed >= 1000)
        {
            _render_fps = _count;
            _count = 0;
            _beg_ms = now;
        }
    }

    switch (frame->format)
    {
    case AV_PIX_FMT_YUV420P:
        return Draw(frame->data[0], frame->linesize[0], // Y
            frame->data[1], frame->linesize[1], // U
            frame->data[2], frame->linesize[2]  // V
        );
    case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ARGB:
        return Draw(frame->data[0], frame->linesize[0]);
    defautl:
        break;
    }
    return false;
}

bool XVideoView::Open(std::string filepath)
{
    if (_ifs.is_open())
    {
        _ifs.close();
    }
    _ifs.open(filepath, ios::binary);
    return _ifs.is_open();
}

AVFrame* XVideoView::Read()
{
    if (_width <= 0 || _height <= 0 || !_ifs) return nullptr;
    // AVFrame空间已经申请，如果参数发生变化，需要释放空间
    if (_frame)
    {
        if (_frame->width != _width
            || _frame->height != _height
            || _frame->format != _fmt)
        {
            // 释放AVFrame对象空间，和buf引用计数减一
            av_frame_free(&_frame);
        }
    }
    if (!_frame)
    {
        // 分配对象空间和像素空间
        _frame = av_frame_alloc();
        _frame->width = _width;
        _frame->height = _height;
        _frame->format = _fmt;
        _frame->linesize[0] = _width * 4;
        if (_frame->format == AV_PIX_FMT_YUV420P)
        {
            _frame->linesize[0] = _width; // Y
            _frame->linesize[1] = _width / 2; // U
            _frame->linesize[2] = _width / 2; // V
        }
        // 生成AVFrame空间，使用默认对齐
        auto re = av_frame_get_buffer(_frame, 0);
        if (re != 0)
        {
            char buf[1024] = { 0 };
            av_strerror(re, buf, sizeof(buf));
            cout << buf << endl;
            av_frame_free(&_frame);
            return nullptr;
        }
    }

    if (!_frame) return nullptr;

    // 读取一帧数据
    if (_frame->format == AV_PIX_FMT_YUV420P)
    {
        _ifs.read((char*)_frame->data[0], _frame->linesize[0] * _frame->height); // Y
        _ifs.read((char*)_frame->data[1], _frame->linesize[1] * _frame->height / 2); // U
        _ifs.read((char*)_frame->data[2], _frame->linesize[2] * _frame->height / 2); // V
    }
    else // RGBA ARGB BGRA 32
    {
        _ifs.read((char*)_frame->data[0], _frame->linesize[0] * _frame->height);
    }
    if (_ifs.gcount() == 0)
        return nullptr;
    return _frame;
}
