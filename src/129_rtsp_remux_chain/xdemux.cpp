#include "xdemux.h"
#include "xtools.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C" 
{
#include <libavformat/avformat.h>
}
#define BERR(err) if (err != 0) { PrintErr(err); return 0; } 

AVFormatContext* XDemux::Open(const char* url)
{
	AVFormatContext* ctx = nullptr;
	// 打开解封装输入上下文

	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); // 使用媒体流为tcp协议，默认为udp
	//av_dict_set(&opts, "stimeout", "1000000", 0); // 连接超时1秒

	auto re = avformat_open_input(&ctx, url,
		NULL,			// 封装器格式 null 自动探测 根据后缀名或者文件头
		&opts			// 参数设置，rtsp需要设置		
	);
	if (opts)
		av_dict_free(&opts);
	BERR(re);

	// 获取媒体信息 无头部格式
	re = avformat_find_stream_info(ctx, NULL);
	BERR(re);
	// 打印封装信息
	av_dump_format(ctx, 0, url, 0);

	return ctx;
}

bool XDemux::Read(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_read_frame(ctx_, pkt);
	BERR(re);
	// 计时，用于超时判断
	last_time_ = NowMs();
	return true;
}

bool XDemux::Seek(long long pts, int stream_index)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_seek_frame(ctx_, stream_index, pts, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	BERR(re);
	return true;
}
