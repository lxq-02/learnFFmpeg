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
	// 400x300 YUV 转 RGBA 800x600 并存到文件中
	int width = 400;
	int height = 300;
	int rgb_width = 800;
	int rgb_height = 600;
	// YUV420P 平面存储 yyyy uu vv
	unsigned char* yuv[3] = { 0 };
	int yuv_linesize[3] = { width , width / 2, width / 2 };
	yuv[0] = new unsigned char[width * height]; // Y
	yuv[1] = new unsigned char[width * height / 4]; // U
	yuv[2] = new unsigned char[width * height / 4]; // V

	// RGBA交叉存储 rgba rgba
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
		// 读取YUV帧
		ifs.read((char*)yuv[0], width * height);
		ifs.read((char*)yuv[1], width * height / 4);
		ifs.read((char*)yuv[2], width * height / 4);
		if (ifs.gcount() == 0) break;

		// YUV转RGBA
		yuv2rgb =  sws_getCachedContext(
			yuv2rgb,				// 转换上下文，NULL新创建，非NULL判断与现有参数是否一致，
									// 一致直接返回，不一致先清理当前然后再创建
			width, height,			// 输入宽高 
			AV_PIX_FMT_YUV420P,		// 输入像素格式
			rgb_width, rgb_height,	// 输出宽高
			AV_PIX_FMT_RGBA,		// 输出像素格式
			SWS_BILINEAR,			// 选择支持的算法，双线性插值
			0, 0, 0					// 过滤器参数
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
			yuv,			// 输入数据
			yuv_linesize,   // 输入数据行字节数
			0,				// 开始处理的行
			height,			// 需要处理的行
			data,			// 输出数据
			lines			// 输出数据步长
		);

		cout << result << " " << flush;

	}

	delete yuv[0];
	delete yuv[1];
	delete yuv[2];
	delete rgba;
	return 0;
}