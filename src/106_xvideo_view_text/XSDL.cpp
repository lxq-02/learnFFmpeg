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
    return true;
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
    if (!_renderer || !_texture || _width <= 0 || _height <= 0)
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
    rect.x = 0;
    rect.y = 0;
    rect.w = _width;
    rect.h = _height;
    result = SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    SDL_RenderPresent(_renderer);


    return false;
}
