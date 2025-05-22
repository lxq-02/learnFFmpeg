#include <iostream>
using namespace std;

extern "C" // 指令函数是C语言函数，函数名不包含重载标注
{
// 引用ffmpeg头文件
#include <libavcodec/avcodec.h>
}

// 预处理指令导入库
#pragma comment(lib, "avcodec.lib")

int main(int argc, char* argv[])
{
	// 1、找到编码器
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		cout << "codec not find!" << endl;
		return -1;
	}

	// 2、编码上下文，设定上下文参数
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cout << "avcodec_alloc_context3 failed!" << endl;
		return -1;
	}
	ctx->width = 400;
	ctx->height = 300;
	// 帧时间戳的时间单位 pts*time_base = 播放时间（秒）
	ctx->time_base = { 1,25 }; // 分数 1/25
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	ctx->thread_count = 16; // 编码线程数，可以通过调用系统接口获取CPU核心数量

	// 3、打开编码器
	int ret = avcodec_open2(ctx, codec, NULL);
	if (ret != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf));
		cout << "avcodec_open2 failed! " << buf << endl;
		return -1;
	}
	cout << "avcodec_open2 success!" << endl;

	// 释放编码上下文
	avcodec_free_context(&ctx);
	return 0;
}