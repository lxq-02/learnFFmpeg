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
	cout << "first ffmpeg " << endl;
	cout << avcodec_configuration() << endl; // 打印ffmpeg版本信息

	// 创建frame对象
	auto frame1 = av_frame_alloc();

	// 图像参数
	frame1->width = 401;
	frame1->height = 300;
	frame1->format = AV_PIX_FMT_ARGB;

	// 分配空间
	int result = av_frame_get_buffer(frame1, 0);
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf));
		cout << buf << endl;
	}

	cout << "frame1 linesize[0] = " << frame1->linesize[0] << endl;
	if (frame1->buf[0])
	{
		cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]); // 线程安全
		cout << endl;
	}

	auto frame2 = av_frame_alloc();
	av_frame_ref(frame2, frame1);
	cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]) << endl;
	cout << "frame2 ref count = " << av_buffer_get_ref_count(frame2->buf[0]) << endl;

	// 引用计数减一，并将buf清空
	av_frame_unref(frame2);
	cout << "av_frame_unref(frame2)" << endl;
	cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]) << endl;

	// 引用计数减一，直接删除buf空间，引用计数变为0
	av_frame_unref(frame1);

	// 释放frame对象，buf的引用计数减一
	// buf已经为空，只删除frame对象空间
	av_frame_free(&frame1);
	av_frame_free(&frame2);
	return 0;
}