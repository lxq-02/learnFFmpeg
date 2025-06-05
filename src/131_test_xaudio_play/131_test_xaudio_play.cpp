#include "xaudio_play.h"
#include <fstream>

#include <iostream>

#undef main
using namespace std;


int main()
{
	auto audio = XAudioPlay::Instance();
	XAudioSpec spec;
	spec.freq = 44100;
	audio->Open(spec);
	ifstream ifs("test.pcm", ios::binary);
	if (!ifs) return -1;
	unsigned char buf[1024] = { 0 };
	for (;;)
	{
		ifs.read((char*)buf, sizeof(buf));
		int len = ifs.gcount();
		if (len <= 0) break; // 读取结束
		audio->Push(buf, len); // 推入音频数据
	}

	getchar();
    return 0;
}

