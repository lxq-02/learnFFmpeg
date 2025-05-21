#include "XVideoView.h"
#include "XSDL.h"
#include <chrono>
#include <iostream>

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
        return Draw(frame->data[0], frame->linesize[0]);
    defautl:
        break;
    }
    return false;
}
