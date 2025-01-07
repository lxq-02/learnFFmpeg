#include "playThread.h"
#include <SDL.h>
#include <qDebug>
#include <QFile>

#define FILENAME "D:/in.wav"

typedef struct AudioBuffer
{
	int len = 0;
	int pullLen = 0;
	Uint8* data = nullptr;
};

playThread::playThread(QObject *parent)
	: QThread(parent)
{
	connect(this, &playThread::finished, this, &playThread::deleteLater);
}

playThread::~playThread()
{
	disconnect();
	isInterruptionRequested();
	quit();
	wait();

	qDebug() << this << QStringLiteral("������");
}

// �ȴ���Ƶ�豸�ص�����ص���Σ�
void pull_audio_data(void* userdata,
						Uint8* stream,	// ��Ҫ��stream�����PCM����
						int len)		// ϣ�����Ĵ�С
{
	qDebug() << "pull_audio_data" << len;

	// ���stream����������
	SDL_memset(stream, 0, len);

	// ȡ��AudioBuffer
	AudioBuffer* buffer = (AudioBuffer*)userdata;

	// ȡlen��bufferLen����Сֵ��Ϊ�˱�֤���ݰ�ȫ����ָֹ��Խ�磩
	if (buffer->len <= 0) return;

	// ȡlen��bufferLen����Сֵ
	buffer->pullLen = (len > buffer->len) ? buffer->len : len;

	// �������
	SDL_MixAudio(stream, (Uint8*)buffer->data, len, SDL_MIX_MAXVOLUME);
	buffer->data += buffer->pullLen;	// ��������ָ�룬�����Ѵ�����ֽ�
	buffer->len -= buffer->pullLen;
}

/*
SDL������Ƶ��2��ģʽ��
Push���ƣ��������������������ݸ�����Ƶ�豸��
Pull������������Ƶ�豸�������򡾳�����ȡ����
*/
void playThread::run()
{
	// ��ʼ�� Audio ��ϵͳ
	if (SDL_Init(SDL_INIT_AUDIO))
	{
		qDebug() << "SDL_Init error" << SDL_GetError();
		return;
	}

	// ����WAV�ļ�
	SDL_AudioSpec spec;
	// ָ��PCM����
	Uint8* data = nullptr;
	// PCM���ݵĳ���
	Uint32 len = 0;
	if (!SDL_LoadWAV(FILENAME, &spec, &data, &len))
	{
		qDebug() << "SDL_LoadWAV error " << SDL_GetError();
		// ������е���ϵͳ
		SDL_Quit();
		return;
	}
	
	// ��Ƶ����������������
	spec.samples = 1024;
	// ���ûص�
	spec.callback = pull_audio_data;
	// ����userdata
	AudioBuffer buffer;
	buffer.data = data;
	buffer.len = len;
	spec.userdata = &buffer;

	if (SDL_OpenAudio(&spec, nullptr))
	{
		qDebug() << "SDL_OpenAudio error" << SDL_GetError();
		// ������е���ϵͳ
		SDL_Quit();
		return;
	}

	// ��ʼ���ţ�0��ȡ����ͣ��
	SDL_PauseAudio(0);

	// ����һЩ����
	int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
	int bytesPerSample = (sampleSize * spec.channels) >> 3;
	while (!isInterruptionRequested())
	{
		// ֻҪ���ļ��ж�ȡ����Ƶ���ݣ���û�������ϣ�������
		if (buffer.len > 0) continue;

		// �ļ������Ѿ���ȡ��
		if (buffer.len <= 0)
		{
			// ʣ�����������
			int samples = buffer.pullLen / bytesPerSample;
			int ms = samples * 1000 / sampleSize;
			SDL_Delay(ms);
			break;
		}
	}

	// �ͷ�wav�ļ�����
	SDL_FreeWAV(data);

	// �ر��豸
	SDL_CloseAudio();

	// ������е���ϵͳ
	SDL_Quit();
}
