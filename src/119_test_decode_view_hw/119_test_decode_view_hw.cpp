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
	// �ָ�h264 ����AVPacket
	// ffmpeg -i test.mp4 -s 400x300 400_300_test.h264
	string filename = "400_300_test.h264";
	ifstream ifs(filename, ios::binary);
	if (!ifs) return -1;
	unsigned char inbuf[4096] = { 0 };

	AVCodecID codec_id = AV_CODEC_ID_H264;

	// 1���ҽ�����
	auto codec = avcodec_find_decoder(codec_id);
	if (!codec)
	{
		cerr << "Codec not found" << endl;
		return -1;
	}

	// 2������������
	auto ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cerr << "Could not allocate codec context" << endl;
		return -1;
	}

	// Ӳ�����ٸ�ʽ DXVA2
	auto hw_type = AV_HWDEVICE_TYPE_DXVA2;
	///////////////////////////////////////
	/// ��ӡ����֧�ֵ�Ӳ�����ٷ�ʽ
	for (int i = 0;; ++i)
	{
		auto config = avcodec_get_hw_config(codec, i);
		if (!config) break;
		if (config->device_type)
			cout << av_hwdevice_get_type_name(config->device_type) << endl;
	}
	// ��ʼ��Ӳ������������
	AVBufferRef* hw_ctx = nullptr;
	av_hwdevice_ctx_create(&hw_ctx, hw_type, nullptr, nullptr, 0);
	ctx->hw_device_ctx = av_buffer_ref(hw_ctx);

	ctx->thread_count = 16;


	// 3����������
	avcodec_open2(ctx, nullptr, nullptr);
	
	// �ָ�������
	AVCodecParserContext* parser = av_parser_init(codec_id);
	auto pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	AVFrame* hw_frame = av_frame_alloc(); // Ӳ����ת����
	auto begin = NowMs();
	int count = 0; // ����ͳ��
	bool is_init_win = false; // �Ƿ��ʼ������

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		if (ifs.eof())
		{
			ifs.clear();
			ifs.seekg(0, ios::beg);
		}
		int data_size = ifs.gcount(); // ��ȡ���ֽ���
		if (data_size <= 0) break;

		auto data = inbuf; // ����ָ��
		while (data_size > 0)	// һ���ж�֡����
		{
			// ͨ��0001 �ض������AVPacket ����֡��С
			int ret = av_parser_parse2(parser, ctx,
				&pkt->data, &pkt->size, // ���
				data, data_size,		// ����
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
			);
			data += ret;
			data_size -= ret; // �Ѵ���
			if (pkt->size)
			{
				//cout << pkt->size << " bytes" << endl;

				// 4������packet�������߳�
				ret = avcodec_send_packet(ctx, pkt);
				if (ret < 0)
				{
					break;
				}


				// 5����ȡ��֡��������
				while (ret >= 0)
				{
					// ÿ�λ����av_frame_unref(frame) ���frame
					ret = avcodec_receive_frame(ctx, frame);
					if (ret < 0)
						break;
					auto pframe = frame; // Ϊ��ͬʱ֧�� Ӳ����������
					if (ctx->hw_device_ctx) // Ӳ����
					{
						// Ӳ����ת��GPU =�� CPU �Դ�=���ڴ�
						av_hwframe_transfer_data(hw_frame, frame, 0);
						pframe = hw_frame;
					}
					cout << frame->format << " " << flush;
					///////////////////////
					/// ��һ֡��ʼ������
					if (!is_init_win)
					{	
						is_init_win = true;
						view->Init(pframe->width, pframe->height, (XVideoView::Format)pframe->format);
					}
					view->DrawFrame(pframe);
					count++;
					auto cur = NowMs();
					if (cur - begin > 1000) // һ���Ӽ���һ�� 
					{
						cout << "\nfps = " << count << endl;
						count = 0;
						begin = cur;
					}
				}
			}
		}
	}

	// ȡ����������
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
