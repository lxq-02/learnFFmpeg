#include "xtools.h"
#include <sstream>
#include <chrono>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

void MSleep(unsigned int ms)
{
	auto beg = chrono::high_resolution_clock::now();
	while (true)
	{
		auto now = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - beg).count();
		if (elapsed >= ms) break;

		// 剩余时间 >1ms 才 sleep
		if (ms - elapsed > 1)
		{
			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
}

long long NowMs()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

void XFreeFrame(AVFrame** frame)
{
	if (!frame || !(*frame)) return;
	av_frame_free(frame);
}


void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

long long XRescale(long long pts, AVRational* src_time_base, AVRational* dst_time_base)
{
	return av_rescale_q(pts, *src_time_base, *dst_time_base);
}


void XThread::Start()
{
	unique_lock<mutex> lock(mtx_);
	static int i = 0;
	i++;
	index_ = i;
	is_exit_ = false;
	// 启动线程
	th_ = thread(&XThread::Main, this);
	stringstream ss;
	ss << "XThread::Start() " << index_;
	LOGINFO(ss.str());
}

void XThread::Stop()
{
	stringstream ss;
	ss << "XThread::Stop() begin " << index_;
	LOGINFO(ss.str());
	is_exit_ = true;



}

void XThread::Wait()
{
	stringstream ss;
	if (th_.joinable()) // 判断子线程是否可以等待
	{
		th_.join();		// 等待子线程退出
	}
	ss.str("");
	ss << "XThread::Stop() " << index_;
	LOGINFO(ss.str());
}

XPara* XPara::Create()
{
	return new XPara();
}

XPara::~XPara()
{
	if (para)
	{
		avcodec_parameters_free(&para);
		para = nullptr;
	}
	if (time_base)
	{
		delete time_base;
		time_base = nullptr;
	}
}

XPara::XPara()
{
	para = avcodec_parameters_alloc();
	time_base = new AVRational();
}

AVPacket* XAVPacketList::Pop()
{
	unique_lock<mutex> lock(mtx_);
	if (pkts_.empty()) return nullptr;
	auto pkt = pkts_.front();
	pkts_.pop_front(); // 出队
	return pkt;
}

void XAVPacketList::Push(AVPacket* pkt)
{
	if (!pkt) return;
	unique_lock<mutex> lock(mtx_);
	// 生成新的AVPacket 对象 引用计数+1
	auto p = av_packet_alloc();
	av_packet_ref(p, pkt); // 引用计数，减少数据复制，线程安全
	pkts_.push_back(p); // 添加到列表中


	// 超出最大空间，清理数据，到关键帧位置
	if (pkts_.size() > max_packets_)
	{
		// 处理第一帧
		if (pkts_.front()->flags && AV_PKT_FLAG_KEY) // 关键帧
		{
			av_packet_free(&pkts_.front());	// 释放内存
			pkts_.pop_front();	// 出队
			return; // 保留关键帧
		}
		// 清理所有非关键帧之前的数据
		while (!pkts_.empty())
		{
			if (pkts_.front()->flags & AV_PKT_FLAG_KEY) // 关键帧
			{
				return; // 保留关键帧
			}
			av_packet_free(&pkts_.front());	// 释放内存
			pkts_.pop_front();	// 出队
		}
	}
}

int XAVPacketList::Size()
{
	unique_lock<mutex> lock(mtx_);
	return pkts_.size();
}

void XAVPacketList::Clear()
{
	unique_lock<mutex> lock(mtx_);
	while (!pkts_.empty())
	{
		av_packet_free(&pkts_.front());
		pkts_.pop_front();
	}
}
