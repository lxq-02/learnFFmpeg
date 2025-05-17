#include "playThread.h"
#include <SDL.h>
#include <qDebug>
#include <QFile>

#define FILENAME "D:/in.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define CHANNELS 2
// ��Ƶ����������������
#define SAMPLES 1024
// ÿ������ռ�ö��ٸ��ֽ�
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// �ļ��������Ĵ�С
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

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

int bufferLen;
char* bufferData;

// �ȴ���Ƶ�豸�ص�����ص���Σ�
void pull_audio_data(void* userdata,
						Uint8* stream,	// ��Ҫ��stream�����PCM����
						int len)		// ϣ�����Ĵ�С
{
	qDebug() << "pull_audio_data" << len;

	// ���stream����������
	SDL_memset(stream, 0, len);

	// ȡlen��bufferLen����Сֵ��Ϊ�˱�֤���ݰ�ȫ����ָֹ��Խ�磩
	if (bufferLen <= 0) return;

	// ȡlen��bufferLen����Сֵ
	len = (len > bufferLen) ? bufferLen : len;

	// �������
	SDL_MixAudio(stream, (Uint8*)bufferData, len, SDL_MIX_MAXVOLUME);
	bufferData += len;	// ��������ָ�룬�����Ѵ�����ֽ�
	bufferLen -= len;
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
	spec.format = AUDIO_S16LSB;
	// ������
	spec.channels = CHANNELS;
	// ��Ƶ���������������������ֵ������2���ݣ�
	spec.samples = 1024;
	// �ص�
	spec.callback = pull_audio_data;
	spec.userdata = (void*)100;

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
	char data[BUFFER_SIZE];
	while (!isInterruptionRequested())
	{
		// ֻҪ���ļ��ж�ȡ����Ƶ���ݣ���û�������ϣ�������
		if (bufferLen > 0) continue;


		bufferLen = file.read(data, BUFFER_SIZE);
		if (bufferLen <= 0) break;

		// ��ȡ�����ļ�����
		bufferData = data;
	}
	//while (!isInterruptionRequested())
	//{
	//	bufferLen = file.read(data, BUFFER_SIZE);
	//	if (bufferLen <= 0) break;

	//	// ��ȡ�����ļ�����
	//	bufferData = data;

	//	// �ȴ���Ƶ����������
	//	// ֻҪ��Ƶ���ݻ�û�������ϣ���Delay(sleep)
	//	while (bufferLen > 0)
	//	{
	//		//SDL_Delay(1);
	//	}
	//}

	// �ر��豸
	SDL_CloseAudio();

	// ������е���ϵͳ
	SDL_Quit();
}
