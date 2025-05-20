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
    // ��ʼ��SDL��Ƶ��
    InitVideo();

    // ȷ���̰߳�ȫ
    unique_lock<mutex> sdl_lock(_mtx);
    _width = w;
    _height = h;
    _fmt = fmt;

    // 1����������
    if (!_window)
    {
        if (!win_id)
        {
            // �½�����
            _window = SDL_CreateWindow("",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                w, h,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        else
        {
            // ��Ⱦ���ؼ�����
            _window = SDL_CreateWindowFrom(win_id);
        }
    }
    if (!_window)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 2��������Ⱦ��
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_renderer)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 3�����������Դ棩
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
        sdl_fmt,                        // ���ظ�ʽ
        SDL_TEXTUREACCESS_STREAMING,    // Ƶ���޸ĵ���Ⱦ��������
        w, h);                          // �����С
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
        case XVideoView::YUV420P: // YUV420P��Planar��YUV���������Ƿֿ��洢��
            linesize = _width;
            break;
        default:
            break;
        }
    }

    if (linesize <= 0)
        return false;
    // �������ݵ������Դ�
    int result = SDL_UpdateTexture(_texture, nullptr, data, linesize);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // �����Ļ
    SDL_RenderClear(_renderer);

    // �����Ƶ���Ⱦ��
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
