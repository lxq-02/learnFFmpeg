#include <iostream>
#include <thread>
#include "xdecode.h"
#include "XVideoView.h"
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

//void PrintErr(int err)
//{
//	char buf[1024] = { 0 };
//	av_strerror(err, buf, sizeof(buf) - 1);
//	cerr << buf << endl;
//}
#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	// 1、打开媒体文件
	const char* url = "test.mp4";
	// 解封装输入上下文
	AVFormatContext* ic = nullptr;
	auto re = avformat_open_input(&ic, url,
		NULL,			// 封装器格式 null 自动探测 根据后缀名或者文件头
		NULL			// 参数设置，rtsp需要设置		
	);
	CERR(re);

	// 2、获取媒体信息 无头部格式
	re = avformat_find_stream_info(ic, NULL);
	CERR(re);

	// 打印封装信息
	av_dump_format(ic, 0, url,
		0 // 0表示上下文是输入 1输出
	);

	AVStream* as = nullptr; // 音频流
	AVStream* vs = nullptr; // 视频流
	for (int i = 0; i < ic->nb_streams; ++i)
	{
		// 音频
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			as = ic->streams[i];
			cout << "=============音频===============" << endl;
			cout << "sample_rate:" << as->codecpar->sample_rate << endl;
		}
		else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vs = ic->streams[i];
			cout << "=============视频===============" << endl;
			cout << "width:" << vs->codecpar->width << endl;
			cout << "height:" << vs->codecpar->height << endl;
		}
	}

	
	auto video_codec_id = vs->codecpar->codec_id;	// 视频编码器ID
	auto video_par = vs->codecpar;					// 视频编码参数
	////////////////////////////////////////////////////////////////////////
	/// 视频解码初始化
	XDecode decode;
	auto decode_c = XCodec::Create(video_codec_id, false);
	// 解封装的视频编码参数，传递给解码上下文
	avcodec_parameters_to_context(decode_c, video_par);

	// 设置到解码器中，线程安全，设置完后decode_c 不在外部中使用
	decode.set_context(decode_c);
	if (!decode.Open())
	{
		cout << "decode Open failed!" << endl;
		return -1;
	}
	// 创建解码输出的空间
	auto frame = decode.CreateFrame();

	//////////////////////////////////////////////////////////////////////
	/// 渲染初始化
	auto view = XVideoView::Create();
	view->Init(video_par->width, video_par->height, (XVideoView::Format)video_par->format);
	//////////////////////////////////////////////////////////////////////

	AVPacket pkt;
	for (;;)
	{
		re = av_read_frame(ic, &pkt);
		CERR(re);
		if (vs && pkt.stream_index == vs->index)
		{
			cout << "视频：";

			// 开始解码视频
			if (decode.Send(&pkt))
			{
				while (decode.Recv(frame))
				{
					cout << "解码视频成功：";
					cout << frame->width << "x" << frame->height << endl;
					view->DrawFrame(frame); // 渲染视频帧到窗口
				}
			}
		}
		else if (as && pkt.stream_index == as->index)
		{
			//cout << "音频：";
		}
		//cout << pkt.pts << " : " << pkt.dts << " : " << pkt.size << endl;
		// 
		av_packet_unref(&pkt);
		this_thread::sleep_for(chrono::milliseconds(100));
	}


	av_frame_free(&frame); // 释放解码输出空间
	avformat_close_input(&ic);

	return 0;
}
