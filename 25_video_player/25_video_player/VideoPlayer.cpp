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
	return _fmtCtx ? round(_fmtCtx->duration * av_q2d(AVRational{ 1, AV_TIME_BASE })) : 0;
}

int VideoPlayer::getTime()
{
	return round(_aTime);
}

void VideoPlayer::setTime(int seekTime)
{
	_seekTime = seekTime;
}

void VideoPlayer::setVolumn(int volumn)
{
	_volumn = volumn;
}

int VideoPlayer::getVolumn()
{
	return _volumn;
}

void VideoPlayer::setMute(bool mute)
{
	_mute = mute;
}

bool VideoPlayer::isMute()
{
	return _mute;
}

#pragma mark - ˽�з���
int VideoPlayer::initDecoder(AVCodecContext** decodeCtx, AVStream** stream, AVMediaType type)
{
	// ����typeѰ������ʵ�����Ϣ
	int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
	RET(av_find_best_stream);

	// ������
	int streamIdx = ret;
	*stream = _fmtCtx->streams[streamIdx];
	if (!*stream)
	{
		qDebug() << "stream is empty";
		return -1;
	}

	// Ϊ��ǰ���ҵ����ʵĽ�����
	AVCodec* decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
	if (!decoder)
	{
		qDebug() << "decoder not found" << (*stream)->codecpar->codec_id;
		return -1;
	}

	// ��ʼ������������
	*decodeCtx = avcodec_alloc_context3(decoder);
	if (!decodeCtx)
	{
		qDebug() << "avcodec_alloc_context3 error";
		return -1;
	}

	// �����п�������������������
	ret = avcodec_parameters_to_context(*decodeCtx, (*stream)->codecpar);
	RET(avcodec_parameters_to_context);

	// �򿪽�����
	ret = avcodec_open2(*decodeCtx, decoder, nullptr);
	RET(avcodec_open2);

	return 0;
}

void VideoPlayer::setState(State state)
{
	if (state == _state) return;
	_state = state;
	emit stateChanged(this);
}

void VideoPlayer::readFile()
{
	// ���ؽ��
	int ret = 0;

	// �������װ�����ġ����ļ�
	ret = avformat_open_input(&_fmtCtx, _filename, nullptr, nullptr);
	END(avformat_open_input);

	// ��������Ϣ
	ret = avformat_find_stream_info(_fmtCtx, nullptr);
	END(avformat_find_stream_info);

	// ��ӡ����Ϣ������̨
	av_dump_format(_fmtCtx, 0, _filename, 0);
	fflush(stderr);

	// ��ʼ����Ƶ��Ϣ
	_hasAudio = initAudioInfo() >= 0;
	// ��ʼ����Ƶ��Ϣ
	_hasVideo = initVideoInfo() >= 0;
	if (!_hasAudio && !_hasVideo)
	{
		fataError();
		return;
	}

	// ����Ϊֹ����ʼ�����
	emit initFinished(this);

	// �ı�״̬
	setState(Playing);

	// ��Ƶ�������̣߳���ʼ����
	SDL_PauseAudio(0);

	// ��Ƶ�������̣߳���ʼ����
	std::thread([this]()
		{
			decodeVideo();
		}).detach();

	// �������ļ��ж�ȡ����
	AVPacket pkt;
	while (_state != Stopped)
	{
		// ����seek����
		if (_seekTime >= 0)
		{
			int streamIdx;
			if (_hasAudio) // ����ʹ����Ƶ������
			{
				streamIdx = _aStream->index;
			}
			else
			{
				streamIdx = _vStream->index;
			}

			// ��ʵʱ�� -> ʱ���
			AVRational timeBase = _fmtCtx->streams[streamIdx]->time_base;
			int64_t ts = _seekTime / av_q2d(timeBase);
			ret = av_seek_frame(_fmtCtx, streamIdx, ts, AVSEEK_FLAG_BACKWARD);

			if (ret < 0) // seekʧ��
			{
				qDebug() << QStringLiteral("seekʧ��") << _seekTime << ts << streamIdx;
				_seekTime = -1;
			}
			else
			{
				qDebug() << QStringLiteral("seek�ɹ�") << _seekTime << ts << streamIdx;
				_vSeekTime = _seekTime;
				_aSeekTime = _seekTime;
				_seekTime = -1;
				// �ָ�ʱ��
				_aTime = 0;
				_vTime = 0;
				// ���֮ǰ��ȡ�����ݰ�
				clearAudioPktList();
				clearVideoPktList();
			}
		}

		int vSize = _vPktList.size();
		int aSize = _aPktList.size();

		if (vSize >= VIDEO_MAX_PKT_SIZE || aSize >= AUDIO_MAX_PKT_SIZE)
		{
			continue;
		}

		ret = av_read_frame(_fmtCtx, &pkt);
		if (ret == 0)
		{
			if (pkt.stream_index == _aStream->index) // ��ȡ��������Ƶ����
			{
				addAudioPkt(pkt);
			}
			else if (pkt.stream_index == _vStream->index) // ��ȡ��������Ƶ����
			{
				addVideoPkt(pkt);
			}
			else // �������ļ���β��
			{
				av_packet_unref(&pkt);
			}
		}
		else if (ret == AVERROR_EOF) // �������ļ���β�� 
		{
			if (vSize == 0 && aSize == 0)
			{
				// ˵���ļ������������
				_fmtCtxCanFree = true;
				break;
			}
		}
		else
		{
			ERROR_BUF;
			qDebug() << "av_read_frame error" << errbuf;
			continue;
		}
	}
	if (_fmtCtxCanFree) // �ļ������������
	{
		stop();
	}
	else
	{
		// ���һ�£�_fmtCtx�����ͷ���
		_fmtCtxCanFree = true;
	}
}

void VideoPlayer::free()
{
	while (_hasAudio && !_aCanFree);
	while (_hasVideo && !_vCanFree);
	while (!_fmtCtxCanFree);
	avformat_close_input(&_fmtCtx);
	_fmtCtxCanFree = false;
	_seekTime = -1;
	freeAudio();
	freeVideo();
}

void VideoPlayer::fataError()
{
	// ���stop�ܹ����óɹ�
	_state = Playing;
	stop();
	emit playFailed(this);
}