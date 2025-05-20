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
	cout << "first ffmpeg " << endl;
	cout << avcodec_configuration() << endl; // ��ӡffmpeg�汾��Ϣ

	// ����frame����
	auto frame1 = av_frame_alloc();

	// ͼ�����
	frame1->width = 401;
	frame1->height = 300;
	frame1->format = AV_PIX_FMT_ARGB;

	// ����ռ�
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
		cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]); // �̰߳�ȫ
		cout << endl;
	}

	auto frame2 = av_frame_alloc();
	av_frame_ref(frame2, frame1);
	cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]) << endl;
	cout << "frame2 ref count = " << av_buffer_get_ref_count(frame2->buf[0]) << endl;

	// ���ü�����һ������buf���
	av_frame_unref(frame2);
	cout << "av_frame_unref(frame2)" << endl;
	cout << "frame1 ref count = " << av_buffer_get_ref_count(frame1->buf[0]) << endl;

	// ���ü�����һ��ֱ��ɾ��buf�ռ䣬���ü�����Ϊ0
	av_frame_unref(frame1);

	// �ͷ�frame����buf�����ü�����һ
	// buf�Ѿ�Ϊ�գ�ֻɾ��frame����ռ�
	av_frame_free(&frame1);
	av_frame_free(&frame2);
	return 0;
}