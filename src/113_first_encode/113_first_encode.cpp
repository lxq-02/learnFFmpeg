#include <iostream>
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
	// 1���ҵ�������
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
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

	// �ͷű���������
	avcodec_free_context(&ctx);
	return 0;
}