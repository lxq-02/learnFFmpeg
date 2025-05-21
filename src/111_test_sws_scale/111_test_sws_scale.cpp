#include <iostream>
#include <fstream>
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")

#define YUV_FILE "400_300_25.yuv"

int main(int argc, char* argv[])
{
	cout << "111_test_sws_scale" << endl;
	// ffmpeg -i test.mp4 -s 400x300 400_300_25.yuv
	// 400x300 YUV ת RGBA 800x600 ���浽�ļ���
	int width = 400;
	int height = 300;
	int rgb_width = 800;
	int rgb_height = 600;
	// YUV420P ƽ��洢 yyyy uu vv
	unsigned char* yuv[3] = { 0 };
	int yuv_linesize[3] = { width , width / 2, width / 2 };
	yuv[0] = new unsigned char[width * height]; // Y
	yuv[1] = new unsigned char[width * height / 4]; // U
	yuv[2] = new unsigned char[width * height / 4]; // V

	// RGBA����洢 rgba rgba
	unsigned char* rgba = new unsigned char[rgb_width * rgb_height * 4];
	int rgba_linesize = rgb_width * 4;

	ifstream ifs;
	ifs.open(YUV_FILE, ios::binary);
	if (!ifs)
	{
		cout << "open " << YUV_FILE << " failed!" << endl;
		return -1;
	}
	SwsContext* yuv2rgb = nullptr;
	for (;;)
	{
		// ��ȡYUV֡
		ifs.read((char*)yuv[0], width * height);
		ifs.read((char*)yuv[1], width * height / 4);
		ifs.read((char*)yuv[2], width * height / 4);
		if (ifs.gcount() == 0) break;

		// YUVתRGBA
		yuv2rgb =  sws_getCachedContext(
			yuv2rgb,				// ת�������ģ�NULL�´�������NULL�ж������в����Ƿ�һ�£�
									// һ��ֱ�ӷ��أ���һ��������ǰȻ���ٴ���
			width, height,			// ������ 
			AV_PIX_FMT_YUV420P,		// �������ظ�ʽ
			rgb_width, rgb_height,	// ������
			AV_PIX_FMT_RGBA,		// ������ظ�ʽ
			SWS_BILINEAR,			// ѡ��֧�ֵ��㷨��˫���Բ�ֵ
			0, 0, 0					// ����������
		);
		if (!yuv2rgb)
		{
			cout << "sws_getCachedContext failed!" << endl;
			return -1;
		}

		unsigned char* data[1];
		data[0] = rgba;
		int lines[1] = { rgba_linesize };
		int result = sws_scale(yuv2rgb,
			yuv,			// ��������
			yuv_linesize,   // �����������ֽ���
			0,				// ��ʼ�������
			height,			// ��Ҫ�������
			data,			// �������
			lines			// ������ݲ���
		);

		cout << result << " " << flush;

	}

	delete yuv[0];
	delete yuv[1];
	delete yuv[2];
	delete rgba;
	return 0;
}