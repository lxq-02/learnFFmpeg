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

	// 复制视频参数
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
	if (!pkt || pkt->stream_index != stream_index_) // 判断是否是视频流
	{
		return;
	}
	pkt_list_.Push(pkt); // 将 AVPacket 添加到列表中
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
			frame_ = av_frame_alloc(); // 分配 AVFrame 内存
	}

	long long cur_pts = -1; // 当前解码到的pts（以解码数据为准）
	while (!is_exit_)
	{
		// 同步
		while (!is_exit_)
		{
			if (syn_pts_ >= 0 && cur_pts > syn_pts_)
			{
				MSleep(1);
				continue;
			}
			break;
		}

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
				cout << "@" << flush;
				need_view_ = true; // 设置需要渲染
				cur_pts = frame_->pts; // 获取当前解码的pts
			}
			if (frame_cache_)
			{
				auto f = av_frame_alloc();
				av_frame_ref(f, frame_); // 引用加一
				frames_.push_back(f); // 将 AVFrame 添加到列表中
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
	auto re = av_frame_ref(f, frame_); // 引用加一
	if (re != 0)
	{
		av_frame_free(&f); // 如果引用失败，释放内存
		PrintErr(re);
		return nullptr;
	}
	need_view_ = false;
	return f;
}
