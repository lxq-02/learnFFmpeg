#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecode_task.h"
#include "xvideo_view.h"
using namespace std;

#define CAM1 "rtsp://127.0.0.1/test"

int main(int argc, char* argv[])
{
	XDemuxTask demux_task;
	for (;;)
	{
		if (demux_task.Open(CAM1))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	auto para = demux_task.CopyVideoPara();
	// ��ʼ����Ⱦ
	auto view = XVideoView::Create();
	view->Init(para->para);

	XDecodeTask decode_task;
	if (!decode_task.Open(para->para))
	{
		LOGERROR("open decode failed!");
	}
	else
	{
		// �趨��һ��������
		demux_task.set_next(&decode_task);
		demux_task.Start();
		decode_task.Start();
	}

	for (;;)
	{
		auto f = decode_task.GetFrame();
		if (!f)
		{
			// û�����ݣ��ȴ�һ��ʱ��
			MSleep(1);
			continue;
		}

		view->DrawFrame(f); // ��Ⱦ֡
		XFreeFrame(&f);
	}


	getchar();
	return 0;
}
