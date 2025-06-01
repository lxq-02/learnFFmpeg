#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecode_task.h"
#include "xvideo_view.h"
#include "xmux_task.h"
using namespace std;

#define CAM1 "rtsp://127.0.0.1/test"
#define CAM2 "test.mp4"

int main(int argc, char* argv[])
{
	XDemuxTask demux_task;
	for (;;)
	{
		if (demux_task.Open(CAM2))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	auto vpara = demux_task.CopyVideoPara();
	AVCodecParameters* video_para = nullptr;
	AVCodecParameters* audio_para = nullptr;
	AVRational* video_time_base = nullptr;
	AVRational* audio_time_base = nullptr;
	if (vpara)
	{
		video_para = vpara->para;
		video_time_base = vpara->time_base;
	}
	auto apara = demux_task.CopyAudioPara();
	if (apara)
	{
		audio_para = apara->para;
		audio_time_base = apara->time_base;
	}
	
	XMuxTask mux_task;
	if (!mux_task.Open("rtsp_out1.mp4", video_para, video_time_base,
		audio_para, audio_time_base
	))
	{
		LOGERROR("mux_task open failed!");
		return -1;
	}
	demux_task.set_next(&mux_task);
	demux_task.Start();
	mux_task.Start();
	MSleep(5000);
	mux_task.Stop();

	if (!mux_task.Open("rtsp_out2.mp4", video_para, video_time_base,
		audio_para, audio_time_base
	))
	{
		LOGERROR("mux_task open failed!");
		return -1;
	}
	mux_task.Start();
	MSleep(5000);
	mux_task.Stop();

	getchar();
	return 0;
}
