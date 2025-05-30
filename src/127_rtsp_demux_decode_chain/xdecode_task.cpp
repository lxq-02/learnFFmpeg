#include "xdecode_task.h"
#include "xtools.h"
#include <iostream>
using namespace std;

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

	// 复制视频参数
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
	cout << "XDecodeTask::Do() begin" << endl;
	if (!pkt || pkt->stream_index != 0) // 判断是否是视频流
	{
		return;
	}
	pkt_list_.Push(pkt); // 将 AVPacket 添加到列表中
}

void XDecodeTask::Main()
{
	{
		unique_lock<mutex> lock(mtx_);
		if (!frame_)
			frame_ = av_frame_alloc(); // 分配 AVFrame 内存
	}


	while (!is_exit_)
	{
		auto pkt = pkt_list_.Pop(); // 从列表中获取 AVPacket
		if (!pkt)
		{
			this_thread::sleep_for(chrono::milliseconds(1)); // 如果没有数据，等待一段时间
			continue;
		}
		// 发送到解码线程
		bool re = decode_.Send(pkt);
		av_packet_free(&pkt); // 释放 AVPacket 内存
		if (!re)
		{
			this_thread::sleep_for(chrono::milliseconds(1)); // 如果发送失败，等待一段时间
			continue;
		}
		{
			unique_lock<mutex> lock(mtx_);
			if (decode_.Recv(frame_)) // 接收解码数据
			{
				cout << "@@@@@@@@@@@@@" << flush;
			}
		}

		this_thread::sleep_for(chrono::milliseconds(1)); // 避免 CPU 占用过高
	}

	{
		unique_lock<mutex> lock(mtx_);
		if (frame_)
		{
			av_frame_unref(frame_); // 清除 AVFrame 的数据
		}
	}
}
