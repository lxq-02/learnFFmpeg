#include "playThread.h"
#include <SDL.h>
#include <qDebug>
#include <QFile>

#define FILENAME "D:/in.pcm"
// ������
#define SAMPLE_RATE 44100
// ������ʽ
#define SAMPLE_FORMAT AUDIO_S16LSB
// ������С
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// ������
#define CHANNELS 2
// ��Ƶ����������������
#define SAMPLES 1024
// ÿ������ռ�ö��ٸ��ֽ�
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
// �ļ��������Ĵ�С
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

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

	// ��Ƶ����
	SDL_AudioSpec spec;
	// ������
	spec.freq = SAMPLE_RATE;
	// ������ʽ��s16le��
	spec.format = SAMPLE_FORMAT;
	// ������
	spec.channels = CHANNELS;
	// ��Ƶ���������������������ֵ������2���ݣ�
	spec.samples = 1024;
	// �ص�
	spec.callback = pull_audio_data;
	// ���ݸ��ص��Ĳ���
	AudioBuffer buffer;
	spec.userdata = &buffer;

	if (SDL_OpenAudio(&spec, nullptr))
	{
		qDebug() << "SDL_OpenAudio error" << SDL_GetError();
		// ������е���ϵͳ
		SDL_Quit();
		return;
	}

	// ���ļ�
	QFile file(FILENAME);
	if (!file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
		// �ر��豸
		SDL_CloseAudio();
		// ������е���ϵͳ
		SDL_Quit();
		return;
	}

	// ��ʼ���ţ�0��ȡ����ͣ��
	SDL_PauseAudio(0);

	// ��Ŵ��ļ��ж�ȡ������
	Uint8 data[BUFFER_SIZE];
	while (!isInterruptionRequested())
	{
		// ֻҪ���ļ��ж�ȡ����Ƶ���ݣ���û�������ϣ�������
		if (buffer.len > 0) continue;

		buffer.len = file.read((char*)data, BUFFER_SIZE);
		// �ļ������Ѿ���ȡ��
		if (buffer.len <= 0)
		{
			// ʣ�����������
			int samples = buffer.pullLen / BYTES_PER_SAMPLE;
			int ms = samples * 1000 / SAMPLE_RATE;
			SDL_Delay(ms);
			break;
		}

		// ��ȡ�����ļ�����
		buffer.data = data;
	}

	// �ر��ļ�
	file.close();
	// �ر��豸
	SDL_CloseAudio();

	// ������е���ϵͳ
	SDL_Quit();
}
