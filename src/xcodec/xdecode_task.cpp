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

	is_open_ = false;
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

	is_open_ = true;
    return true;
}

void XDecodeTask::Stop()
{
	pkt_list_.Clear();

	unique_lock<mutex> lock(mtx_);
	decode_.set_context(nullptr);
	is_open_ = false;

	while (!frames_.empty())
	{
		av_frame_free(&frames_.front());
		frames_.pop_front();
	}
}

void XDecodeTask::Do(AVPacket* pkt)
{
	cout << "#" << flush;
	if (!pkt || pkt->stream_index != stream_index_) // �ж��Ƿ�����Ƶ��
	{
		return;
	}
	pkt_list_.Push(pkt); // �� AVPacket ��ӵ��б���
	if (block_size_ <= 0) return;
	while (!is_exit_)
	{
		if (pkt_list_.Size() > block_size_)
		{
			MSleep(1);
			continue;
		}
		break;
	}
}

void XDecodeTask::Main()
{
	{
		unique_lock<mutex> lock(mtx_);
		if (!frame_)
			frame_ = av_frame_alloc(); // ���� AVFrame �ڴ�
	}

	long long cur_pts = -1; // ��ǰ���뵽��pts���Խ�������Ϊ׼��
	while (!is_exit_)
	{
		// ͬ��
		while (!is_exit_)
		{
			if (syn_pts_ >= 0 && cur_pts > syn_pts_)
			{
				MSleep(1);
				continue;
			}
			break;
		}

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
				cur_pts = frame_->pts; // ��ȡ��ǰ�����pts
			}
			if (frame_cache_)
			{
				auto f = av_frame_alloc();
				av_frame_ref(f, frame_); // ���ü�һ
				frames_.push_back(f); // �� AVFrame ��ӵ��б���
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
	if (frame_cache_)
	{
		if (frames_.empty()) return nullptr;
		auto f = frames_.front();
		frames_.pop_front();
		return f;
	}

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
