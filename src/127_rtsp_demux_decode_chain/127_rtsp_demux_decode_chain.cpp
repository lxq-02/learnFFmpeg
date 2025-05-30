#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecode_task.h"
using namespace std;

#define CAM1 "rtsp://127.0.0.1:8554/"

int main(int argc, char* argv[])
{
	XDemuxTask demux_task;
	for (;;)
	{
		if (demux_task.Open(CAM1))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	auto para = demux_task.CopyVideoPara();
	XDecodeTask decode_task;
	if (!decode_task.Open(para->para))
	{
		LOGERROR("open decode failed!");
	}
	else
	{
		// 设定下一个责任链
		demux_task.set_next(&decode_task);
		demux_task.Start();
		decode_task.Start();
	}


	getchar();
	return 0;
}
