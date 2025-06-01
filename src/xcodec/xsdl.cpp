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
    // ���������㷨������������, ���Բ�ֵ
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    return true;
}

void XSDL::Close()
{
    // ȷ���̰߳�ȫ
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
    // ��ʼ��SDL��Ƶ��
    InitVideo();

    // ȷ���̰߳�ȫ
    unique_lock<mutex> sdl_lock(mtx_);
    width_ = w;
    height_ = h;
    fmt_ = fmt;

    // 1����������
    if (!window_)
    {
        if (!win_id_)
        {
            // �½�����
            window_ = SDL_CreateWindow("",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                w, h,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        else
        {
            // ��Ⱦ���ؼ�����
            window_ = SDL_CreateWindowFrom(win_id_);
        }
    }
    if (!window_)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 2��������Ⱦ��
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_)
    {
        cerr << SDL_GetError() << endl;
        return false;
    }

    // 3�����������Դ棩
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
		sdl_fmt = SDL_PIXELFORMAT_NV12; // NV12��YUV��ʽ
        break;
    default:
        break;
    }
    if (texture_)
        SDL_DestroyTexture(texture_);
    texture_ = SDL_CreateTexture(renderer_,
        sdl_fmt,                        // ���ظ�ʽ
        SDL_TEXTUREACCESS_STREAMING,    // Ƶ���޸ĵ���Ⱦ��������
        w, h);                          // �����С
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
        case XVideoView::YUV420P: // YUV420P��Planar��YUV���������Ƿֿ��洢��
            linesize = width_;
            break;
        default:
            break;
        }
    }

    if (linesize <= 0)
        return false;
    // �������ݵ������Դ�
    int result = SDL_UpdateTexture(texture_, nullptr, data, linesize);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // �����Ļ
    SDL_RenderClear(renderer_);

    // �����Ƶ���Ⱦ��
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (scale_w_ > 0) // �û��ֶ���������
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = scale_w_; // ��Ⱦ�Ŀ�ߣ�������
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
    // �������
    if (!y || !u || !v) return false;
    if (!renderer_ || !texture_ || !window_ || width_ <= 0 || height_ <= 0)
        return false;

    // �������ݵ������Դ�
    int result = SDL_UpdateYUVTexture(texture_, nullptr, 
        y, y_pitch,
        u, u_pitch,
        v, v_pitch);
    if (result != 0)
    {
        cout << SDL_GetError() << endl;
        return false;
    }

    // �����Ļ
    SDL_RenderClear(renderer_);

    // �����Ƶ���Ⱦ��
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (scale_w_ > 0) // �û��ֶ���������
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = scale_w_; // ��Ⱦ�Ŀ�ߣ�������
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
