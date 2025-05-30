#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
using namespace std;

#define CAM1 "rtsp://127.0.0.1:8554/"

int main(int argc, char* argv[])
{
	XDemuxTask det;
	for (;;)
	{
		if (det.Open(CAM1))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	det.Start();

	getchar();
	return 0;
}
