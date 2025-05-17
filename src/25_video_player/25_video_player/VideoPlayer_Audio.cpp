#include "VideoPlayer.h"
#include <QDebug>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

int VideoPlayer::initAudioInfo()
{
	// ��ʼ��������
	int ret = initDecoder(&_aDecodecCtx, &_aStream, AVMEDIA_TYPE_AUDIO);
	RET(initDecoder);

	// ��ʼ����Ƶ�ز���
	ret = initSwr();
	RET(initSwr);

	// ��ʼ��SDL
	ret = initSDL();
	RET(initSDL);

	return 0;
}

int VideoPlayer::initSwr()
{
	// �ز����������
	_aSwrInSpec.sampleFmt = _aDecodecCtx->sample_fmt;
	_aSwrInSpec.sampleRate = _aDecodecCtx->sample_rate;
	_aSwrInSpec.chLayout = _aDecodecCtx->channel_layout;
	_aSwrInSpec.chs = _aDecodecCtx->channels;

	// �ز����������
	_aSwrOutSpec.sampleFmt = AV_SAMPLE_FMT_S16;
	_aSwrOutSpec.sampleRate = 44100;
	_aSwrOutSpec.chLayout = AV_CH_LAYOUT_STEREO;
	_aSwrOutSpec.chs = av_get_channel_layout_nb_channels(_aSwrOutSpec.chLayout);
	_aSwrOutSpec.bytesPerSampleFrame = _aSwrOutSpec.chs
		* av_get_bytes_per_sample(_aSwrOutSpec.sampleFmt);

	// �����ز���������
	_aSwrCtx = swr_alloc_set_opts(nullptr,
		// �������
		_aSwrOutSpec.chLayout,
		_aSwrOutSpec.sampleFmt,
		_aSwrOutSpec.sampleRate,
		// �������
		_aSwrInSpec.chLayout,
		_aSwrInSpec.sampleFmt,
		_aSwrInSpec.sampleRate,
		0, nullptr);
	if (!_aSwrCtx)
	{
		qDebug() << "swr_alloc_set_opts error";
		return -1;
	}

	// ��ʼ���ز���������
	int ret = swr_init(_aSwrCtx);
	RET(swr_init);

	// ��ʼ���ز���������frame
	_aSwrInFrame = av_frame_alloc();
	if (!_aSwrInFrame)
	{
		qDebug() << "av_frame_alloc error";
		return -1;
	}

	// ��ʼ���ز��������frame
	_aSwrOutFrame = av_frame_alloc();
	if (!_aSwrOutFrame)
	{
		qDebug() << "av_frame_alloc error";
		return -1;
	}

	// _aSwrOutFrame��data[0]ָ����ڴ�ռ�
	ret = av_samples_alloc(_aSwrOutFrame->data,
		_aSwrOutFrame->linesize,
		_aSwrOutSpec.chs,
		4096, _aSwrOutSpec.sampleFmt, 1);
	RET(av_samples_alloc);

	return 0;
}

int VideoPlayer::initSDL()
{
	// ��Ƶ����
	SDL_AudioSpec spec;
	// ������
	spec.freq = _aSwrOutSpec.sampleRate;
	// ������ʽ��s16le)
	spec.format = AUDIO_S16LSB;
	// ������
	spec.channels = _aSwrOutSpec.chs;
	// ��Ƶ���������������������ֵ������2���ݣ�
	spec.samples = 512;
	// �ص�
	spec.callback = sdlAudioCallbackFunc;
	// ���ݸ��ص��Ĳ���
	spec.userdata = this;

	// ����Ƶ�豸
	if (SDL_OpenAudio(&spec, nullptr))
	{
		qDebug() << "SDL_OpenAudio error" << SDL_GetError();
		return -1;
	}

	return 0;
}

// ���ã����Ŷ������µ�ָ��
void VideoPlayer::addAudioPkt(AVPacket& pkt)
{
	_aMutex.lock();
	_aPktList.push_back(pkt);
	_aMutex.signal();
	_aMutex.unlock();
}

void VideoPlayer::clearAudioPktList()
{
	_aMutex.lock();
	for (AVPacket& pkt : _aPktList)
	{
		av_packet_unref(&pkt);
	}
	_aPktList.clear();
	_aMutex.unlock();
}

void VideoPlayer::freeAudio()
{
	_aTime = 0;
	_aSwrOutIdx = 0;
	_aSwrOutSize = 0;
	_aStream = nullptr;
	_aCanFree = false;
	_aSeekTime = -1;

	clearAudioPktList();
	avcodec_free_context(&_aDecodecCtx);
	swr_free(&_aSwrCtx);
	av_frame_free(&_aSwrInFrame);
	if (_aSwrOutFrame)
	{
		av_freep(&_aSwrOutFrame->data[0]);
		av_frame_free(&_aSwrOutFrame);
	}

	// ֹͣ����
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void VideoPlayer::sdlAudioCallbackFunc(void* userdata, Uint8* stream, int len)
{
	VideoPlayer* player = (VideoPlayer*)userdata;
	player->sdlAudioCallback(stream, len);
}

void VideoPlayer::sdlAudioCallback(Uint8* stream, int len)
{
	// ���㣨������
	SDL_memset(stream, 0, len);

	// len��SDL��Ƶ������ʣ��Ĵ�С����δ���Ĵ�С��
	while (len > 0)
	{
		if (_state == Paused) break;
		if (_state == Stopped)
		{
			_aCanFree = true;
			break;
		}

		// ˵����ǰPCM�������Ѿ�ȫ��������SDL����Ƶ��������
		// ��Ҫ������һ��pkt����ȡ�µ�PCM����
		if (_aSwrOutIdx >= _aSwrOutSize)
		{
			// ȫ�µ�PCM��С
			_aSwrOutSize = decodeAudio();
			// ������0
			_aSwrOutIdx = 0;
			// û�н����PCM���ݣ��Ǿ;�������
			if (_aSwrOutSize <= 0)
			{
				// �ٶ�PCM�Ĵ�С
				_aSwrOutSize = 1024;
				// ��PCM���0��������
				memset(_aSwrOutFrame->data[0], 0, _aSwrOutSize);
			}
		}

		// ������Ҫ��䵽stream��PCM���ݴ�С
		int fillLen = _aSwrOutSize - _aSwrOutIdx;
		fillLen = std::min(fillLen, len);

		// ��ȡ��ǰ����
		int volumn = _mute ? 0 : ((_volumn * 1.0 / Max) * SDL_MIX_MAXVOLUME);

		// ���SDL������
		SDL_MixAudio(stream,
			_aSwrOutFrame->data[0] + _aSwrOutIdx,
			fillLen, volumn);

		// �ƶ�ƫ����
		len -= fillLen;
		stream += fillLen;
		_aSwrOutIdx += fillLen;
	}
}

int VideoPlayer::decodeAudio()
{
	// ����
	_aMutex.lock();

	if (_aPktList.empty())
	{
		_aMutex.unlock();
		return 0;
	}

	// ȡ��ͷ�������ݰ�
	AVPacket pkt = _aPktList.front();
	// ��ͷ����ɾ��
	_aPktList.pop_front();
	// ����
	_aMutex.unlock();
	
	// ������Ƶʱ��
	if (pkt.pts != AV_NOPTS_VALUE)
	{
		_aTime = av_q2d(_aStream->time_base) * pkt.pts;
		// ֪ͨ��磺����ʱ��㷢���˸ı�
		emit timeChanged(this);
	}

	// �������Ƶ�����������λ���жϣ�������ǰ�ͷ�pkt����Ȼ�ᵼ��B֡��P֡����ʧ�ܣ�����˺�ѣ�
	// ������Ƶ��ʱ��������seekTime�ģ�ֱ�Ӷ���
	if (_aSeekTime >= 0)
	{
		if (_aTime < _aSeekTime)
		{
			// �ͷ�pkt
			av_packet_unref(&pkt);
			return 0;
		}
		else
		{
			_aSeekTime = -1;
		}
	}

	// ����ѹ�����ݵ�������
	int ret = avcodec_send_packet(_aDecodecCtx, &pkt);
	// �ͷ�pkt
	av_packet_unref(&pkt);
	RET(avcodec_send_packet);
}