#include "xcamera_record.h"
#include "xdemux_task.h"
#include "xmux_task.h"


void XCameraRecord::Main()
{
	XDemuxTask demux;
	XMuxTask mux;
	if (rtsp_url.empty())
	{
		LOGERROR("open rtsp url failed!");
		return;
	}

	// 自动重连
	while (!is_exit_)
	{
		if (demux.Open(rtsp_url.c_str())) // 最坏情况阻塞1秒
		{
			break;
		}
		MSleep(10);
		continue;
	}

	// 音视频参数
	auto vpara = demux.CopyVideoPara();
	if (!vpara)
	{
		LOGERROR("demux.CopyVideoPara failed!");
		// 需要考虑demux资源释放的问题
		demux.Stop();
		return;
	}
	// 启动解封装线程，提前启动，防止超时
	demux.Start();
	auto apara = demux.CopyAudioPara();

	AVCodecParameters* para = nullptr;	// 音频参数
	AVRational* timebase = nullptr;		// 音频时间基数
	if (apara)
	{
		para = apara->para;
		timebase = apara->time_base;
	}

	if (!mux.Open(save_path.c_str(),
		vpara->para, vpara->time_base, // 视频参数
		para, timebase // 音频参数	
	))
	{
		LOGERROR("mux.Open rtsp_url failed!");
		demux.Stop();
		mux.Stop();
		return;
	}
	demux.set_next(&mux);
	mux.Start();
	MSleep(10000);																					
	mux.Stop();
	demux.Stop();
}
