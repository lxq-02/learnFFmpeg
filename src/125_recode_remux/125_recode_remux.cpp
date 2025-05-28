#include <iostream>
#include <thread>
#include "xdemux.h"
#include "xmux.h"
#include "xdecode.h"
#include "xencode.h"
using namespace std;


#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	/////////////////////////////////////////////////////////////////////////////
	/// 输入参数处理
	string useage = "124_test_xformat 输入文件 输出文件 开始时间 结束时间（秒）视频宽 视频高\n";
	useage += "124_test_xformat test.mp4 test_out.mp4 10 20 400 300";

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

	int video_width = 0;
	int video_height = 0;
	if (argc > 6)
	{
		video_width = atoi(argv[5]); // 视频宽度
		video_height = atoi(argv[6]); // 视频高度
	}

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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 解码的初始化
	XDecode decode;
	auto decode_c = decode.Create(demux.video_codec_id(), false);
	// 设置视频解码器参数
	demux.CopyPara(demux.video_index(), decode_c);

	decode.set_context(decode_c);
	decode.Open();
	auto frame = decode.CreateFrame(); // 解码后存储

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// 视频编码初始化
	if (demux.video_index() >= 0)
	{
		if (video_width <= 0)
			video_width = demux_c->streams[demux.video_index()]->codecpar->width;
		if (video_height <= 0)
			video_height = demux_c->streams[demux.video_index()]->codecpar->height; // 视频宽高
	}
	XEncode encode;
	auto encode_c = encode.Create(AV_CODEC_ID_H265, true);
	encode_c->pix_fmt = AV_PIX_FMT_YUV420P;
	encode_c->width = video_width;
	encode_c->height = video_height;
	encode.set_context(encode_c);
	encode.Open();

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
		//demux.CopyPara(demux.video_index(), mvs->codecpar); // 复制视频流参数
		// 复制编码器格式
		avcodec_parameters_from_context(mvs->codecpar, encode_c);
	}
	// 有音频
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num; // 音频流时间基准，与原音频一致
		mas->time_base.den = demux.audio_time_base().den; // 音频流时间基准，与原音频一致
		demux.CopyPara(demux.audio_index(), mas->codecpar); // 复制音频流参数
	}

	// 写入头部，会改变time_base
	mux.WriteHead();

	////////////////////////////////////////////////////////////////////////////

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

		if (pkt.stream_index == demux.video_index())	// 视频
		{
			mux.RescaleTime(&pkt, video_begin_pts, demux.video_time_base());
			// 解码视频
			if (decode.Send(&pkt))
			{
				while (decode.Recv(frame))
				{
					cout << "." << flush;
					// 修改图像尺寸
					// 视频编码
					auto epkt = encode.Encode(frame);
					if (epkt)
					{
						// 写入视频帧，会清理pkt
						mux.Write(epkt);
						av_packet_free(&epkt);
					}
				}
			}
			video_count++;
			if (demux.video_time_base().den > 0)
				total_sec += pkt.duration * (double)demux.video_time_base().num / (double)demux.video_time_base().den; // 累计视频时长
			av_packet_unref(&pkt);
		}
		else if (pkt.stream_index == demux.audio_index())
		{
			mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
			audio_count++;
			// 写入音频帧 会清理pkt
			mux.Write(&pkt);
		}
		else
		{
			av_packet_unref(&pkt);
		}

	}

	mux.WriteEnd(); // 使用XMux类写入尾部信息
	demux.set_ctx(nullptr); // 清理XDemux类中的上下文
	mux.set_ctx(nullptr);
	encode.set_context(nullptr); // 清理XEncode类中的上下文  
	decode.set_context(nullptr); // 清理XDecode类中的上下文
	cout << "输出文件：" << out_file << " : " << endl;
	cout << "视频帧：" << video_count << endl;
	cout << "音频帧：" << audio_count << endl;
	cout << "总时长：" << total_sec << endl;

	return 0;
}
