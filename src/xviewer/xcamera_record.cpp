#include "xcamera_record.h"
#include "xdemux_task.h"
#include "xmux_task.h"


void XCameraRecord::Main()
{
	XDemuxTask demux;
	XMuxTask mux;
	if (rtsp_url.empty())
	{
		LOGERROR("open rtsp url failed!");
		return;
	}

	// �Զ�����
	while (!is_exit_)
	{
		if (demux.Open(rtsp_url.c_str())) // ��������1��
		{
			break;
		}
		MSleep(10);
		continue;
	}

	// ����Ƶ����
	auto vpara = demux.CopyVideoPara();
	if (!vpara)
	{
		LOGERROR("demux.CopyVideoPara failed!");
		// ��Ҫ����demux��Դ�ͷŵ�����
		demux.Stop();
		return;
	}
	// �������װ�̣߳���ǰ��������ֹ��ʱ
	demux.Start();
	auto apara = demux.CopyAudioPara();

	AVCodecParameters* para = nullptr;	// ��Ƶ����
	AVRational* timebase = nullptr;		// ��Ƶʱ�����
	if (apara)
	{
		para = apara->para;
		timebase = apara->time_base;
	}

	if (!mux.Open(save_path.c_str(),
		vpara->para, vpara->time_base, // ��Ƶ����
		para, timebase // ��Ƶ����	
	))
	{
		LOGERROR("mux.Open rtsp_url failed!");
		demux.Stop();
		mux.Stop();
		return;
	}
	demux.set_next(&mux);
	mux.Start();
	MSleep(10000);																					
	mux.Stop();
	demux.Stop();
}
