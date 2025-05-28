#include "xdemux.h"
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
	auto re = avformat_open_input(&ctx, url,
		NULL,			// 封装器格式 null 自动探测 根据后缀名或者文件头
		NULL			// 参数设置，rtsp需要设置		
	);
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
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = av_read_frame(_ctx, pkt);
	BERR(re);
	return true;
}
