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

	qDebug() << this << QStringLiteral("析构了");
}

// 等待音频设备回调（会回调多次）
void pull_audio_data(void* userdata,
						Uint8* stream,	// 需要往stream中填充PCM数据
						int len)		// 希望填充的大小
{
	qDebug() << "pull_audio_data" << len;

	// 清空stream（静音处理）
	SDL_memset(stream, 0, len);

	// 取出AudioBuffer
	AudioBuffer* buffer = (AudioBuffer*)userdata;

	// 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
	if (buffer->len <= 0) return;

	// 取len、bufferLen的最小值
	buffer->pullLen = (len > buffer->len) ? buffer->len : len;

	// 填充数据
	SDL_MixAudio(stream, (Uint8*)buffer->data, len, SDL_MIX_MAXVOLUME);
	buffer->data += buffer->pullLen;	// 更新数据指针，跳过已处理的字节
	buffer->len -= buffer->pullLen;
}

/*
SDL播放音频有2种模式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉取数据
*/
void playThread::run()
{
	// 初始化 Audio 子系统
	if (SDL_Init(SDL_INIT_AUDIO))
	{
		qDebug() << "SDL_Init error" << SDL_GetError();
		return;
	}

	// 加载WAV文件
	SDL_AudioSpec spec;
	// 指向PCM数据
	Uint8* data = nullptr;
	// PCM数据的长度
	Uint32 len = 0;
	if (!SDL_LoadWAV(FILENAME, &spec, &data, &len))
	{
		qDebug() << "SDL_LoadWAV error " << SDL_GetError();
		// 清除所有的子系统
		SDL_Quit();
		return;
	}
	
	// 音频缓冲区的样本数量
	spec.samples = 1024;
	// 设置回调
	spec.callback = pull_audio_data;
	// 设置userdata
	AudioBuffer buffer;
	buffer.data = data;
	buffer.len = len;
	spec.userdata = &buffer;

	if (SDL_OpenAudio(&spec, nullptr))
	{
		qDebug() << "SDL_OpenAudio error" << SDL_GetError();
		// 清除所有的子系统
		SDL_Quit();
		return;
	}

	// 开始播放（0是取消暂停）
	SDL_PauseAudio(0);

	// 计算一些参数
	int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
	int bytesPerSample = (sampleSize * spec.channels) >> 3;
	while (!isInterruptionRequested())
	{
		// 只要从文件中读取的音频数据，还没有填充完毕，就跳过
		if (buffer.len > 0) continue;

		// 文件数据已经读取完
		if (buffer.len <= 0)
		{
			// 剩余的样本数量
			int samples = buffer.pullLen / bytesPerSample;
			int ms = samples * 1000 / sampleSize;
			SDL_Delay(ms);
			break;
		}
	}

	// 释放wav文件数据
	SDL_FreeWAV(data);

	// 关闭设备
	SDL_CloseAudio();

	// 清除所有的子系统
	SDL_Quit();
}
