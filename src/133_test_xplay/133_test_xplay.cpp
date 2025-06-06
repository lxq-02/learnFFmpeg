#include <iostream>
#include "xplayer.h"

#undef main
using namespace std;
#pragma comment(lib, "xcodec.lib")

int main()
{
	XPlayer play;
	play.Open("v1080.mp4", nullptr);
	play.Start();

	for (;;)
	{
		play.Update();
		MSleep(10);
	}

	getchar();
    return 0;
}

