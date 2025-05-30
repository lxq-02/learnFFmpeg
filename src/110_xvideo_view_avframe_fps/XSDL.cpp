#include "XSDL.h"

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
    unique_lock<mutex>  sdl_lock(_mtx);
    if (_texture)
    {
        SDL_DestroyTexture(_texture);
        _texture = nullptr;
    }
    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    if (_window)
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
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

bool XSDL::Init(int w, int h, Format fmt, void* win_id)
{
    if (w <= 0 || h <= 0) return false;
    // 初始化SDL视频库
    InitVideo();

    // 确保线程安全
    unique_lock<mutex> sdl_lock(_mtx);
    _width = w;
    _height = h;
    _fmt = fmt;

    // 1、创建窗口
    if (!_window)
    {
        if (!win_id)
        {
            // 新建窗口
            _window = SDL_CreateWindow("",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                w, h,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        else
        {
            // 渲染到控件窗口
            _window = SDL_CreateWindowFrom(win_id);
        }
    }
    if (!_window)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 2、创建渲染器
    if (_renderer)
        SDL_DestroyRenderer(_renderer);
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_renderer)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 3、创建纹理（显存）
    auto sdl_fmt = SDL_PIXELFORMAT_ABGR8888;
    switch (_fmt)
    {
    case YUV420P:
        sdl_fmt = SDL_PIXELFORMAT_IYUV;
        break;
    default:
        break;
    }
    if (_texture)
        SDL_DestroyTexture(_texture);
    _texture = SDL_CreateTexture(_renderer,
        sdl_fmt,                        // 像素格式
        SDL_TEXTUREACCESS_STREAMING,    // 频繁修改的渲染（带锁）
        w, h);                          // 纹理大小
    if (!_texture)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    return false;
}

bool XSDL::Draw(const unsigned char* data, int linesize)
{
    if (!data) return false;
    if (!_renderer || !_texture || !_window || _width <= 0 || _height <= 0)
        return false;

    if (linesize <= 0)
    {
        switch (_fmt)
        {
        case XVideoView::RGBA:
        case XVideoView::ARGB:
            linesize = _width * 4;
            break;
        case XVideoView::YUV420P: // YUV420P是Planar，YUV三种数据是分开存储的
            linesize = _width;
            break;
        default:
            break;
        }
    }

    if (linesize <= 0)
        return false;
    // 复制数据到纹理，显存
    int result = SDL_UpdateTexture(_texture, nullptr, data, linesize);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // 清空屏幕
    SDL_RenderClear(_renderer);

    // 纹理复制到渲染器
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (_scale_w > 0) // 用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = _scale_w; // 渲染的宽高，可缩放
        rect.h = _scale_h;
        prect = &rect;
    }
    result = SDL_RenderCopy(_renderer, _texture, nullptr, prect);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    SDL_RenderPresent(_renderer);


    return true;
}

bool XSDL::Draw(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch)
{
    // 参数检查
    if (!y || !u || !v) return false;
    if (!_renderer || !_texture || !_window || _width <= 0 || _height <= 0)
        return false;

    // 复制数据到纹理，显存
    int result = SDL_UpdateYUVTexture(_texture, nullptr, 
        y, y_pitch,
        u, u_pitch,
        v, v_pitch);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // 清空屏幕
    SDL_RenderClear(_renderer);

    // 纹理复制到渲染器
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (_scale_w > 0) // 用户手动设置缩放
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = _scale_w; // 渲染的宽高，可缩放
        rect.h = _scale_h;
        prect = &rect;
    }
    result = SDL_RenderCopy(_renderer, _texture, nullptr, prect);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    SDL_RenderPresent(_renderer);


    return false;
}
