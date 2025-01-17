#include "VideoPlayer.h"
#include <thread>
#include <QDebug>
#include <cmath>

extern "C"
{
#include <libavutil/avutil.h>
}

#define AUDIO_MAX_PKT_SIZE 1000
#define VIDEO_MAX_PKT_SIZE 500

#pragma mark - 构造、析构
VideoPlayer::VideoPlayer(QObject* parent)
	:QObject(parent)
{
	// 初始化Audio子系统
	if (SDL_Init(SDL_INIT_AUDIO))
	{
		// 返回值不是0，就代表失败
		qDebug() << "SDL_Init error" << SDL_GetError();
		emit playFailed(this);
		return;
	}
}

VideoPlayer::~VideoPlayer()
{
	// 不再对外发送信息
	disconnect();

	stop();

	SDL_Quit();
}

#pragma mark - 公共方法
void VideoPlayer::play()
{
	if (_state == Playing) return;
	// 状态可能是：暂停、停止、播放完毕

	if (_state == Stopped)
	{
		// 开始线程：读取文件
		std::thread([this]()
			{
				readFile();
			});
	}
	else
	{
		// 改变zhuangt
		setState(Playing);
	}
}

void VideoPlayer::pause()
{
	if (_state != Playing) return;
	// 状态可能是：正在播放

	// 改变状态
	setState(Paused);
}

void VideoPlayer::stop()
{
	if (_state == Stopped) return;
	// 状态可能是：正在播放、暂停、正常完毕

	// 改变状态
	//setState(Stopped);
	_state = Stopped;
	
	// 释放资源
	free();

	// 通知外界
	emit stateChanged(this);
}

bool VideoPlayer::isPlaying()
{
	return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState()
{
	return _state;
}

void VideoPlayer::setFilename(QString& filename)
{
	const char* name = filename.toStdString().c_str();
	memcpy(_filename, name, strlen(name) + 1);
}

int VideoPlayer::getDuration()
{
	return _fmtCtx ? round(_fmtCtx->duration * av_q2d(AV_TIME_BASE_Q) : 0);
}

int VideoPlayer::getTime()
{
	return 0;
}

void VideoPlayer::setTime(int seekTime)
{
}

void VideoPlayer::setVolumn(int volumn)
{
}

int VideoPlayer::getVolumn()
{
	return 0;
}

void VideoPlayer::setMute(bool mute)
{
}

void VideoPlayer::isMute()
{
}
