#include "YuvPlayer.h"
#include <vector>

extern "C"
{
#include <libavutil/imgutils.h>
}

#define RET(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		return; \
	}

static const std::map<AVPixelFormat, SDL_PixelFormatEnum>
PIXEL_FORMAT_MAP =
{
	{AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
	{AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
	{AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN}
};

YuvPlayer::YuvPlayer(QWidget* parent)
	:QWidget(parent)
{
	// ��������
	_window = SDL_CreateWindowFrom((void*)winId());
	RET(!_window, SDL_CreateWindowFrom);

	// ������Ⱦ������
	// ���Դ�����������Ⱦ������
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!_renderer)
	{
		// ����ΪĬ����Ⱦ������
		_renderer = SDL_CreateRenderer(_window, -1, 0);
		RET(!_renderer, SDL_CreateRenderer);
	}
}

YuvPlayer::~YuvPlayer()
{
	_file.close();
	SDL_DestroyTexture(_texture);
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
}

void YuvPlayer::play()
{
	_timerId = startTimer(1000 / _yuv.fps);
	_state = YuvPlayer::Playing;
}

void YuvPlayer::pause()
{
	if (_timerId)
	{
		killTimer(_timerId);
	}
	_state = YuvPlayer::Paused;
}

void YuvPlayer::stop()
{
	if (_timerId)
	{
		killTimer(_timerId);
	}
	_state = YuvPlayer::Stopped;
}

bool YuvPlayer::isPlaying()
{
	return _state == YuvPlayer::Playing;
}

void YuvPlayer::setYUV(Yuv& yuv)
{
	_yuv = yuv;

	// ��������
	_texture = SDL_CreateTexture(_renderer,
		yuv.pixelFormat,
		PIXEL_FORMAT_MAP.find(yuv.pixelFormat)->second,
		yuv.width,
		yuv.height);
	RET(!_texture, SDL_CreateTexture);

	// ���ļ�
	_file.setFileName(yuv.filename);
	if (!_file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << yuv.filename;
	}
}

YuvPlayer::State YuvPlayer::getState()
{
	return _state;
}

void YuvPlayer::timerEvent(QTimerEvent* event)
{
	// ͼƬ��С
	int imgSize = av_image_get_buffer_size(_yuv.pixelFormat,
											_yuv.width,
											_yuv.height,
											1);

	std::vector<char> data(imgSize);
	if (_file.read(data.data(), imgSize) > 0)
	{
		// ��YUV������������䵽texture
		RET(SDL_UpdateTexture(_texture, nullptr, data.data(), _yuv.width),
			SDL_UpdateTexture);

		// ���û�����ɫ��������ɫ��
		RET(SDL_SetRenderDrawColor(_renderer,
			0, 0, 0, SDL_ALPHA_OPAQUE),
			SDL_GetRenderDrawColor);

		// �û�����ɫ��������ɫ�������ȾĿ��
		RET(SDL_RenderClear(_renderer),
			SDL_RenderClear);

		// �����������ݵ���ȾĿ�꣨Ĭ����window)
		RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr),
			SDL_RenderCopy);

		// �������е���Ⱦ��������Ļ��
		SDL_RenderPresent(_renderer);
	}
	else
	{
		// �ļ������Ѿ���ȡ���
		killTimer(_timerId);
	}
}
