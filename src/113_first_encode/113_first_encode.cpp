#include <iostream>
#include <fstream>
using namespace std;

extern "C" // ָ�����C���Ժ��������������������ر�ע
{
// ����ffmpegͷ�ļ�
#include <libavcodec/avcodec.h>
}

// Ԥ����ָ����
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



	// 1���ҵ�������
	AVCodec* codec = avcodec_find_encoder(codec_id);
	if (!codec)
	{
		cout << "codec not find!" << endl;
		return -1;
	}

	// 2�����������ģ��趨�����Ĳ���
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
	{
		cout << "avcodec_alloc_context3 failed!" << endl;
		return -1;
	}
	ctx->width = 400;
	ctx->height = 300;
	// ֡ʱ�����ʱ�䵥λ pts*time_base = ����ʱ�䣨�룩
	ctx->time_base = { 1,25 }; // ���� 1/25
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	ctx->thread_count = 16; // �����߳���������ͨ������ϵͳ�ӿڻ�ȡCPU��������

	// 3���򿪱�����
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
	// ʮ����Ƶ 250֡
	for (int i = 0; i < 250; ++i)
	{
		// ����AVFrame���� ÿ֡���ݲ�ͬ
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
		frame->pts = i; // ��ʾ��ʱ��

		// ����δѹ��֡���߳���ѹ��
		ret = avcodec_send_frame(ctx, frame);
		if (ret != 0)
		{
			break;
		}
		
		while (ret >= 0) // �п���ȡ����֡����
		{
			// ����ѹ��֡��һ��ǰ���ε��÷��ؿգ����壬���̷��أ�����δ��ɣ�
			// �������ڶ������߳���
			// ÿ�ε��û����·���pkt�еĿռ�
			ret = avcodec_receive_packet(ctx, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			if (ret < 0) // ������
			{
				char buf[1024] = { 0 };
				av_strerror(ret, buf, sizeof(buf));
				cout << "avcodec_receive_packet failed! " << buf << endl;
				break;
			}
			cout << pkt->size << " " << flush;
			ofs.write((char*)pkt->data, pkt->size);
			av_packet_unref(pkt); // �ͷ��ڲ���data
		}
	}
	ofs.close();
	av_packet_free(&pkt);
	av_frame_free(&frame);


	
	// �ͷű���������
	avcodec_free_context(&ctx);
	return 0;
}