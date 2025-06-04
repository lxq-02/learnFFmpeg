#include "xcamera_record.h"
#include "xdemux_task.h"
#include "xmux_task.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
using namespace std;
using namespace chrono;

/**
 * 生成存储的视频文件名
 * 
 * @return 
 */
static std::string GetFileName(std::string path)
{
	stringstream ss;
	// 获取当前时间戳
	auto t = system_clock::to_time_t(system_clock::now());
	std::tm tm_time{};
	localtime_s(&tm_time, &t); // 将时间戳转换为本地时间
	auto time_str = put_time(&tm_time, "%Y_%m_%d_%H_%M_%S");
	ss << path << "/" << "cam_" << time_str << ".mp4"; // 假设保存为mp4格式
	return ss.str();
}


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
		MSleep(3000);
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

	// 打开了封装
	if (!mux.Open(GetFileName(save_path).c_str(),
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

	// 当前时间
	auto cur = NowMs();

	while (!is_exit_)
	{
		if (NowMs() - cur > file_sec_ * 1000)
		{
			cur = NowMs();
			mux.Stop(); // 停止存储，写入索引

			// 打开了封装
			if (!mux.Open(GetFileName(save_path).c_str(),
				vpara->para, vpara->time_base, // 视频参数
				para, timebase // 音频参数	
			))
			{
				LOGERROR("mux.Open rtsp_url failed!");
				demux.Stop();
				mux.Stop();
				return;
			}
			mux.Start();
		}
		// 定时创建新的文件
		MSleep(10);
	}
	mux.Stop();
	demux.Stop();
}
