#include "xdecode_task.h"
#include "xtools.h"
#include <iostream>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

bool XDecodeTask::Open(AVCodecParameters* para)
{
	if (!para)
	{
		LOGERROR("para is null");
		return false;
	}
	unique_lock<mutex> lock(mtx_);
	auto c = decode_.Create(para->codec_id, false);
	if (!c)
	{
		LOGERROR("decode_.Create failed!");
		return false;
	}

	// ������Ƶ����
	avcodec_parameters_to_context(c, para);
	decode_.set_context(c);
	if (!decode_.Open())
	{
		LOGERROR("decode_.Open failed!");
		return false;
	}
	LOGINFO("Open decode success!");

    return true;
}

void XDecodeTask::Do(AVPacket* pkt)
{
	cout << "#" << endl;
	if (!pkt || pkt->stream_index != 0) // �ж��Ƿ�����Ƶ��
	{
		return;
	}
	pkt_list_.Push(pkt); // �� AVPacket ��ӵ��б���
}

void XDecodeTask::Main()
{
	{
		unique_lock<mutex> lock(mtx_);
		if (!frame_)
			frame_ = av_frame_alloc(); // ���� AVFrame �ڴ�
	}


	while (!is_exit_)
	{
		auto pkt = pkt_list_.Pop(); // ���б��л�ȡ AVPacket
		if (!pkt)
		{
			this_thread::sleep_for(chrono::milliseconds(1)); // ���û�����ݣ��ȴ�һ��ʱ��
			continue;
		}
		// ���͵������߳�
		bool re = decode_.Send(pkt);
		av_packet_free(&pkt); // �ͷ� AVPacket �ڴ�
		if (!re)
		{
			this_thread::sleep_for(chrono::milliseconds(1)); // �������ʧ�ܣ��ȴ�һ��ʱ��
			continue;
		}
		{
			unique_lock<mutex> lock(mtx_);
			if (decode_.Recv(frame_)) // ���ս�������
			{
				cout << "@" << flush;
				need_view_ = true; // ������Ҫ��Ⱦ
			}
		}

		this_thread::sleep_for(chrono::milliseconds(1)); // ���� CPU ռ�ù���
	}

	{
		unique_lock<mutex> lock(mtx_);
		if (frame_)
		{
			av_frame_unref(frame_); // ��� AVFrame ������
		}
	}
}

AVFrame* XDecodeTask::GetFrame()
{
	unique_lock<mutex> lock(mtx_);
	if (!need_view_ || !frame_ || !frame_->buf[0]) return nullptr;
	auto f = av_frame_alloc();
	auto re = av_frame_ref(f, frame_); // ���ü�һ
	if (re != 0)
	{
		av_frame_free(&f); // �������ʧ�ܣ��ͷ��ڴ�
		PrintErr(re);
		return nullptr;
	}
	need_view_ = false;
	return f;
}
