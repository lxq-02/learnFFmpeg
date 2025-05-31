#include "xsdl.h"

#include <SDL.h>
#include <iostream>
using namespace std;
#pragma comment(lib, "SDL2.lib")

static bool InitVideo()
{
    static bool is_first = true;
    static mutex mux;
    unique_lock<mutex> sdl_lock(mux);
    if (!is_first) return true;
    is_first = false;
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        cout << SDL_GetError() << endl;
        return false;
    }
    // 设置缩放算法，解决锯齿问题, 线性插值
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    return true;
}

void XSDL::Close()
{
    // 确保线程安全
    unique_lock<mutex>  sdl_lock(mtx_);
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

}

bool XSDL::IsExit()
{
    SDL_Event ev;
    SDL_WaitEventTimeout(&ev, 1);
    if (ev.type == SDL_QUIT)
        return true;
    return false;
}

bool XSDL::Init(int w, int h, Format fmt)
{
    if (w <= 0 || h <= 0) return false;
    // 初始化SDL视频库
    InitVideo();

    // 确保线程安全
    unique_lock<mutex> sdl_lock(mtx_);
    width_ = w;
    height_ = h;
    fmt_ = fmt;

    // 1、创建窗口
    if (!window_)
    {
        if (!win_id_)
        {
            // 新建窗口
            window_ = SDL_CreateWindow("",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                w, h,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        else
        {
            // 渲染到控件窗口
            window_ = SDL_CreateWindowFrom(win_id_);
        }
    }
    if (!window_)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 2、创建渲染器
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 3、创建纹理（显存）
    auto sdl_fmt = SDL_PIXELFORMAT_ABGR8888;
    switch (fmt_)
    {
    case XVideoView::RGBA:
        sdl_fmt = SDL_PIXELFORMAT_RGBA32;
        break;
    case XVideoView::BGRA:
        sdl_fmt = SDL_PIXELFORMAT_BGRA32;
        break;
    case XVideoView::ARGB:
        sdl_fmt = SDL_PIXELFORMAT_ARGB32;
        break;
    case XVideoView::YUV420P:
    case XVideoView::YUVJ422P:
        sdl_fmt = SDL_PIXELFORMAT_IYUV;
        break;
    case XVideoView::RGB24:
        sdl_fmt = SDL_PIXELFORMAT_RGB24;
        break;
    case XVideoView::NV12:
		sdl_fmt = SDL_PIXELFORMAT_NV12; // NV12是YUV格式
        break;
    default:
        break;
    }
    if (texture_)
        SDL_DestroyTexture(texture_);
    texture_ = SDL_CreateTexture(renderer_,
        sdl_fmt,                        // 像素格式
        SDL_TEXTUREACCESS_STREAMING,    // 频繁修改的渲染（带锁）
        w, h);                          // 纹理大小
    if (!texture_)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    return false;
}

bool XSDL::Draw(const unsigned char* data, int linesize)
{
    if (!data) return false;
    if (!renderer_ || !texture_ || !window_ || width_ <= 0 || height_ <= 0)
        return false;

    if (linesize <= 0)
    {
        switch (fmt_)
        {
        case XVideoView::RGBA:
        case XVideoView::ARGB:
            linesize = width_ * 4;
            break;
        case XVideoView::YUV420P: // YUV420P是Planar，YUV三种数据是分开存储的
            linesize = width_;
            break;
        default:
            break;
        }
    }

    if (linesize <= 0)
        return false;
    // 复制数据到纹理，显存
    int result = SDL_UpdateTexture(texture_, nullptr, data, linesize);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // 清空屏幕
    SDL_RenderClear(renderer_);

    // 纹理复制到渲染器
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (scale_w_ > 0) // 用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = scale_w_; // 渲染的宽高，可缩放
        rect.h = scale_h_;
        prect = &rect;
    }
    result = SDL_RenderCopy(renderer_, texture_, nullptr, prect);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    SDL_RenderPresent(renderer_);


    return true;
}

bool XSDL::Draw(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch)
{
    // 参数检查
    if (!y || !u || !v) return false;
    if (!renderer_ || !texture_ || !window_ || width_ <= 0 || height_ <= 0)
        return false;

    // 复制数据到纹理，显存
    int result = SDL_UpdateYUVTexture(texture_, nullptr, 
        y, y_pitch,
        u, u_pitch,
        v, v_pitch);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // 清空屏幕
    SDL_RenderClear(renderer_);

    // 纹理复制到渲染器
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (scale_w_ > 0) // 用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = scale_w_; // 渲染的宽高，可缩放
        rect.h = scale_h_;
        prect = &rect;
    }
    result = SDL_RenderCopy(renderer_, texture_, nullptr, prect);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    SDL_RenderPresent(renderer_);


    return false;
}
