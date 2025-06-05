#include <iostream>
#include <fstream>
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")
#undef main
using namespace std;

#define FILENAME "./test.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define CHANNELS 2
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// 文件缓冲区的大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

int bufferLen;
char* bufferData;

// 等待音频设备回调（会回调多次）
void AudioCallback(void* userdata,
						Uint8* stream,	// 需要往stream中填充PCM数据
						int len)		// 希望填充的大小
{
	cout << "AudioCallback" << endl;

	// 清空stream（静音处理）
	SDL_memset(stream, 0, len);

	auto ifs = (ifstream*)userdata;	// 获取用户数据（文件流）
	ifs->read((char*)stream, len);
	if (ifs->gcount() <= 0)
	{
		SDL_PauseAudio(1);	// 暂停音频设备
	}
}

int main()
{
    // 初始化 SDL 音频模块
    SDL_Init(SDL_INIT_AUDIO);

    // 打开音频设备
	SDL_AudioSpec spec;				// 音频参数
	spec.freq = SAMPLE_RATE;		// 采样率
	spec.format = AUDIO_S16LSB;		// 采样格式（s16le）
	spec.channels = CHANNELS;		// 声道数（1单声道，2立体声）
	spec.silence = 0;				// 静音值（通常为0）
	spec.samples = 1024;			// 音频缓冲区的样本数量（这个值必须是2的幂）
									// 用于分割音频数据的大小，通常是1024或2048
									// 例如：samples = 8
									//       LLLL RRRR
	spec.callback = AudioCallback;	// 回调函数（当音频设备需要更多数据时会调用）
	ifstream ifs(FILENAME, ios::binary);	// 二进制打开
	if (!ifs)
	{
		cerr << "open pcm file failed!" << endl;
		return -1;
	}
	spec.userdata = &ifs;

	if (SDL_OpenAudio(&spec, nullptr) < 0)
	{
		std::cerr << "SDL_OpenAudio error" << SDL_GetError() << endl;
		// 清除所有的子系统
		SDL_Quit();
		return -1;
	}

	// 开始播放（0是取消暂停）
	SDL_PauseAudio(0);


	getchar();

	SDL_QuitSubSystem(SDL_INIT_AUDIO);


    return 0;
}

