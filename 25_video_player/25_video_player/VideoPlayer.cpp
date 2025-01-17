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

#pragma mark - ���졢����
VideoPlayer::VideoPlayer(QObject* parent)
	:QObject(parent)
{
	// ��ʼ��Audio��ϵͳ
	if (SDL_Init(SDL_INIT_AUDIO))
	{
		// ����ֵ����0���ʹ���ʧ��
		qDebug() << "SDL_Init error" << SDL_GetError();
		emit playFailed(this);
		return;
	}
}

VideoPlayer::~VideoPlayer()
{
	// ���ٶ��ⷢ����Ϣ
	disconnect();

	stop();

	SDL_Quit();
}

#pragma mark - ��������
void VideoPlayer::play()
{
	if (_state == Playing) return;
	// ״̬�����ǣ���ͣ��ֹͣ���������

	if (_state == Stopped)
	{
		// ��ʼ�̣߳���ȡ�ļ�
		std::thread([this]()
			{
				readFile();
			});
	}
	else
	{
		// �ı�zhuangt
		setState(Playing);
	}
}

void VideoPlayer::pause()
{
	if (_state != Playing) return;
	// ״̬�����ǣ����ڲ���

	// �ı�״̬
	setState(Paused);
}

void VideoPlayer::stop()
{
	if (_state == Stopped) return;
	// ״̬�����ǣ����ڲ��š���ͣ���������

	// �ı�״̬
	//setState(Stopped);
	_state = Stopped;
	
	// �ͷ���Դ
	free();

	// ֪ͨ���
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
