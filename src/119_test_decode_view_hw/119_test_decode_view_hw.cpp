#include <iostream>
#include <string>
#include <fstream>
#include "XVideoView.h"
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

int main(int argc, char* argv[])
{
	auto view = XVideoView::Create();
	// 分割h264 存入AVPacket
	// ffmpeg -i test.mp4 -s 400x300 400_300_test.h264
	string filename = "400_300_test.h264";
	ifstream ifs(filename, ios::binary);
	if (!ifs) return -1;
	unsigned char inbuf[4096] = { 0 };

	AVCodecID codec_id = AV_CODEC_ID_H264;

	// 1、找解码器
	auto codec = avcodec_find_decoder(codec_id);
	if (!codec)
	{
		cerr << "Codec not found" << endl;
		return -1;
	}

	// 2、创建上下文
	auto ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cerr << "Could not allocate codec context" << endl;
		return -1;
	}

	// 硬件加速格式 DXVA2
	auto hw_type = AV_HWDEVICE_TYPE_DXVA2;
	///////////////////////////////////////
	/// 打印所有支持的硬件加速方式
	for (int i = 0;; ++i)
	{
		auto config = avcodec_get_hw_config(codec, i);
		if (!config) break;
		if (config->device_type)
			cout << av_hwdevice_get_type_name(config->device_type) << endl;
	}
	// 初始化硬件加速上下文
	AVBufferRef* hw_ctx = nullptr;
	av_hwdevice_ctx_create(&hw_ctx, hw_type, nullptr, nullptr, 0);
	ctx->hw_device_ctx = av_buffer_ref(hw_ctx);

	ctx->thread_count = 16;


	// 3、打开上下文
	avcodec_open2(ctx, nullptr, nullptr);
	
	// 分割上下文
	AVCodecParserContext* parser = av_parser_init(codec_id);
	auto pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	AVFrame* hw_frame = av_frame_alloc(); // 硬解码转换用
	auto begin = NowMs();
	int count = 0; // 解码统计
	bool is_init_win = false; // 是否初始化窗口

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		if (ifs.eof())
		{
			ifs.clear();
			ifs.seekg(0, ios::beg);
		}
		int data_size = ifs.gcount(); // 读取的字节数
		if (data_size <= 0) break;

		auto data = inbuf; // 数据指针
		while (data_size > 0)	// 一次有多帧数据
		{
			// 通过0001 截断输出到AVPacket 返回帧大小
			int ret = av_parser_parse2(parser, ctx,
				&pkt->data, &pkt->size, // 输出
				data, data_size,		// 输入
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
			);
			data += ret;
			data_size -= ret; // 已处理
			if (pkt->size)
			{
				//cout << pkt->size << " bytes" << endl;

				// 4、发送packet到解码线程
				ret = avcodec_send_packet(ctx, pkt);
				if (ret < 0)
				{
					break;
				}


				// 5、获取多帧解码数据
				while (ret >= 0)
				{
					// 每次会调用av_frame_unref(frame) 清空frame
					ret = avcodec_receive_frame(ctx, frame);
					if (ret < 0)
						break;
					auto pframe = frame; // 为了同时支持 硬解码和软解码
					if (ctx->hw_device_ctx) // 硬解码
					{
						// 硬解码转换GPU =》 CPU 显存=》内存
						av_hwframe_transfer_data(hw_frame, frame, 0);
						pframe = hw_frame;
					}
					cout << frame->format << " " << flush;
					///////////////////////
					/// 第一帧初始化窗口
					if (!is_init_win)
					{	
						is_init_win = true;
						view->Init(pframe->width, pframe->height, (XVideoView::Format)pframe->format);
					}
					view->DrawFrame(pframe);
					count++;
					auto cur = NowMs();
					if (cur - begin > 1000) // 一秒钟计算一次 
					{
						cout << "\nfps = " << count << endl;
						count = 0;
						begin = cur;
					}
				}
			}
		}
	}

	// 取出缓存数据
	int ret = avcodec_send_packet(ctx, nullptr);
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(ctx, frame);
		if (ret < 0)
			break;
		cout << frame->format << "-" << flush;
	}

	av_parser_close(parser);
	avcodec_free_context(&ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return 0;
}
