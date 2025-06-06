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

		// ʣ��ʱ�� >1ms �� sleep
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
	// �����߳�
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
	if (th_.joinable()) // �ж����߳��Ƿ���Եȴ�
	{
		th_.join();		// �ȴ����߳��˳�
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
	pkts_.pop_front(); // ����
	return pkt;
}

void XAVPacketList::Push(AVPacket* pkt)
{
	if (!pkt) return;
	unique_lock<mutex> lock(mtx_);
	// �����µ�AVPacket ���� ���ü���+1
	auto p = av_packet_alloc();
	av_packet_ref(p, pkt); // ���ü������������ݸ��ƣ��̰߳�ȫ
	pkts_.push_back(p); // ��ӵ��б���


	// �������ռ䣬�������ݣ����ؼ�֡λ��
	if (pkts_.size() > max_packets_)
	{
		// �����һ֡
		if (pkts_.front()->flags && AV_PKT_FLAG_KEY) // �ؼ�֡
		{
			av_packet_free(&pkts_.front());	// �ͷ��ڴ�
			pkts_.pop_front();	// ����
			return; // �����ؼ�֡
		}
		// �������зǹؼ�֮֡ǰ������
		while (!pkts_.empty())
		{
			if (pkts_.front()->flags & AV_PKT_FLAG_KEY) // �ؼ�֡
			{
				return; // �����ؼ�֡
			}
			av_packet_free(&pkts_.front());	// �ͷ��ڴ�
			pkts_.pop_front();	// ����
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
