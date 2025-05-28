#include <iostream>
#include <thread>
#include "xdemux.h"
#include "xmux.h"
using namespace std;


#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	/////////////////////////////////////////////////////////////////////////////
	/// 输入参数处理
	string useage = "124_test_xformat 输入文件 输出文件 开始时间 结束时间（秒）\n";
	useage += "124_test_xformat test.mp4 test_out.mp4 10 20";

	cout << useage << endl;
	string input_file = "";
	string out_file = "";
	if (argc < 3)
	{
		return -1;
	}
	input_file = argv[1]; // 输入文件
	out_file = argv[2]; // 输出文件

	/// 截取10~20秒之间的音视频帧 取多不取少
	// 假定9 11秒有关键帧，我们取第9秒的
	int begin_sec = 0;	// 截取开始时间
	int end_sec = 0;		// 截取结束时间
	if (argc > 3)
		begin_sec = atoi(argv[3]); // 开始时间
	if (argc > 4)
		end_sec = atoi(argv[4]); // 结束时间

	////////////////////////////////////////////////////////////////////

	//// 1、打开媒体文件
	//const char* url = "test.mp4";

	/////////////////////////////////////////////////////////////////////
	/// 解封装
	// 解封装输入上下文
	XDemux demux; // 使用XDemux类
	auto demux_c = demux.Open(input_file.c_str());

	demux.set_ctx(demux_c); // 设置上下文到XDemux类中

	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// 封装
	// 上下文
	//const char* out_url = "test_mux.mp4";

	XMux mux; // 使用XMux类
	auto mux_c = mux.Open(out_file.c_str()); // 打开封装输出上下文
	mux.set_ctx(mux_c);
	auto mvs = mux_c->streams[mux.video_index()]; // 视频流
	auto mas = mux_c->streams[mux.audio_index()]; // 音频流

	// 有视频
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num; // 视频流时间基准，与原视频一致
		mvs->time_base.den = demux.video_time_base().den; // 视频流时间基准，与原视频一致
		demux.CopyPara(demux.video_index(), mvs->codecpar); // 复制视频流参数
	}
	// 有音频
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num; // 音频流时间基准，与原音频一致
		mas->time_base.den = demux.audio_time_base().den; // 音频流时间基准，与原音频一致
		demux.CopyPara(demux.audio_index(), mas->codecpar); // 复制音频流参数
	}

	mux.WriteHead();

	////////////////////////////////////////////////////////////////////////////
	
	long long video_begin_pts = 0;
	long long audio_begin_pts = 0; // 音频流的pts
	long long video_end_pts = 0;

	// 开始截断秒数 算出输入视频的pts
	if (begin_sec > 0)
	{
		// 计算视频的开始和计数时间
		if (demux.video_index() >= 0 && demux.video_time_base().num > 0) // 有视频流
		{
			double t = (double)demux.video_time_base().den / (double)demux.video_time_base().num; // 1秒的帧数
			video_begin_pts = (long long)(begin_sec * t); // 视频流的pts，单位为AV_TIME_BASE
			video_end_pts = (long long)(end_sec * t); // 视频流的结束pts，单位为AV_TIME_BASE
			demux.Seek(video_begin_pts, demux.video_index()); // seek到视频流的pts位置 开始帧
		}

		// 计算音频的开始播放pts
		if (demux.audio_index() >= 0 && demux.audio_time_base().num > 0) // 有视频流
		{
			double t = (double)demux.audio_time_base().den / (double)demux.audio_time_base().num; // 1秒的帧数
			audio_begin_pts = (long long)(begin_sec * t); // 音频流的pts，单位为AV_TIME_BASE
			demux.Seek(audio_begin_pts, demux.audio_index()); // seek到视频流的pts位置 开始帧
		}

	}

	int audio_count = 0;
	int video_count = 0;
	double total_sec = 0;
	AVPacket pkt;
	for (;;)
	{
		if (!demux.Read(&pkt)) // 使用XDemux类读取
		{
			break; // 读取结束
		}

		// 视频 时间大于结束时间
		if (video_end_pts > 0 
			&& pkt.stream_index == demux.video_index() // 视频流索引
			&& pkt.pts > video_end_pts)
		{
			av_packet_unref(&pkt);
			break;
		}

		if (pkt.stream_index == demux.video_index())
		{
			mux.RescaleTime(&pkt, video_begin_pts, demux.video_time_base());
			video_count++;
			if (demux.video_time_base().den > 0)
				total_sec += pkt.duration * (double)demux.video_time_base().num / (double)demux.video_time_base().den; // 累计视频时长
		}
		else if (pkt.stream_index == demux.audio_index())
		{
			mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
			audio_count++;
		}

		// 写入音视频帧 会清理pkt
		mux.Write(&pkt); // 使用XMux类写入
		//av_packet_unref(&pkt);
		//this_thread::sleep_for(chrono::milliseconds(100));
	}

	mux.WriteEnd(); // 使用XMux类写入尾部信息
	demux.set_ctx(nullptr); // 清理XDemux类中的上下文
	mux.set_ctx(nullptr);
	cout << "输出文件：" << out_file << " : " << endl;
	cout << "视频帧：" << video_count << endl;
	cout << "音频帧：" << audio_count << endl;
	cout << "总时长：" << total_sec << endl;

	return 0;
}
