#include <iostream>
#include <fstream>
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
	string filename = "400_300_25";
	AVCodecID codec_id = AV_CODEC_ID_H264;
	if (argc > 1)
	{
		string codec = argv[1];
		if (codec == "h265" || codec == "hevc")
		{
			codec_id = AV_CODEC_ID_HEVC;
		}
	}
	if (codec_id == AV_CODEC_ID_H264)
	{
		filename += ".h264";
	}
	else if (codec_id == AV_CODEC_ID_H265)
	{
		filename += ".h265";
	}
	ofstream ofs;
	ofs.open(filename, ios::binary);



	// 1、找到编码器
	AVCodec* codec = avcodec_find_encoder(codec_id);
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

	AVFrame *frame = av_frame_alloc();
	frame->width = ctx->width;
	frame->height = ctx->width;
	frame->format = ctx->pix_fmt;
	ret = av_frame_get_buffer(frame, 0);
	if (ret != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf));
		cout << "av_frame_get_buffer failed! " << buf << endl;
		return -1;
	}
	AVPacket* pkt = av_packet_alloc();
	// 十秒视频 250帧
	for (int i = 0; i < 250; ++i)
	{
		// 生成AVFrame数据 每帧数据不同
		// Y
		for (int y = 0; y < ctx->height; ++y)
		{
			for (int x = 0; x < ctx->width; ++x)
			{
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}
		// UV
		for (int y = 0; y < ctx->height / 2; ++y)
		{
			for (int x = 0; x < ctx->width / 2; ++x)
			{
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}
		frame->pts = i; // 显示的时间

		// 发送未压缩帧到线程中压缩
		ret = avcodec_send_frame(ctx, frame);
		if (ret != 0)
		{
			break;
		}
		
		while (ret >= 0) // 有可能取出多帧数据
		{
			// 接收压缩帧，一般前几次调用返回空（缓冲，立刻返回，编码未完成）
			// 编码是在独立的线程中
			// 每次调用会重新分配pkt中的空间
			ret = avcodec_receive_packet(ctx, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			if (ret < 0) // 出错了
			{
				char buf[1024] = { 0 };
				av_strerror(ret, buf, sizeof(buf));
				cout << "avcodec_receive_packet failed! " << buf << endl;
				break;
			}
			cout << pkt->size << " " << flush;
			ofs.write((char*)pkt->data, pkt->size);
			av_packet_unref(pkt); // 释放内部的data
		}
	}
	ofs.close();
	av_packet_free(&pkt);
	av_frame_free(&frame);


	
	// 释放编码上下文
	avcodec_free_context(&ctx);
	return 0;
}