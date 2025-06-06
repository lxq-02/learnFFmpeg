#pragma once

// ����Linux _WIN32 windows 32λ��64λ
#ifdef _WIN32
#ifdef XCODEC_EXPORTS
#define XCODEC_API __declspec(dllexport)
#else
#define XCODEC_API __declspec(dllimport)
#endif
#else
#define XCODEC_API 
#endif

#include <thread>
#include <iostream>
#include <list>
#include <mutex>
struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFrame;
struct AVCodecContext;

// ��־���� DEBUG INFO ERROR FATAL
enum XLogLevel
{
	XLOG_TYPE_DEBUG,
	XLOG_TYPE_INFO,
	XLOG_TYPE_ERROR,
	XLOG_TYPE_FATAL
};

#define LOG_MIN_LEVEL XLOG_TYPE_DEBUG
#define XLOG(s, level) \
if (level >= LOG_MIN_LEVEL) \
	std::cout << level << ":" << __FILE__ << ":\n" << __LINE__ << s << std::endl;
#define LOGDEBUG(s) XLOG(s, XLOG_TYPE_DEBUG)
#define LOGINFO(s) XLOG(s, XLOG_TYPE_INFO)
#define LOGERROR(s) XLOG(s, XLOG_TYPE_ERROR)
#define LOGFATAL(s) XLOG(s, XLOG_TYPE_FATAL)


XCODEC_API void MSleep(unsigned int ms);

// ��ȡ��ǰʱ��� ����
XCODEC_API long long NowMs();

// �ͷ�AVFrame����ռ䣬��buf���ü�����һ
XCODEC_API void XFreeFrame(AVFrame** frame);

XCODEC_API void PrintErr(int err);

// ����ʱ���������
XCODEC_API long long XRescale(long long pts,
	AVRational* src_time_base,
	AVRational* dst_time_base);

class XCODEC_API XThread
{
public:
	// �����߳�
	virtual void Start();

	// �����߳��˳���־
	virtual void Exit();

	// ֹͣ�߳� �ȴ��˳�
	virtual void Stop();

	// �ȴ��߳��˳�
	virtual void Wait();

	// ִ��������Ҫ����
	virtual void Do(AVPacket* pkt) {}

	// ���ݵ���һ������������
	virtual void Next(AVPacket* pkt)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (next_)
		{
			next_->Do(pkt);
		}
		else
		{
			LOGDEBUG("XThread::Next() no next thread!");
		}
	}

	// ��������������һ���ڵ㣨�̰߳�ȫ��
	void set_next(XThread* xt)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		next_ = xt;
	}
	// ��ͣ�򲥷�
	virtual void Pause(bool is_pause) { is_pause_ = is_pause; }

	bool is_pause() { return is_pause_; }
protected:

	// �߳���ں���
	virtual void Main() = 0; 
	// ��־�߳��˳�
	bool is_exit_ = false;	// �õ������ж��Ƿ��˳���

	// �߳�������
	int index_ = 0;



private:
	bool is_pause_ = false;			// �Ƿ�����ͣ״̬
	std::thread th_;
	std::mutex mtx_;
	XThread *next_ = nullptr;	// ����������һ���ڵ�
};

class XTools
{
};

// ����Ƶ����
class XCODEC_API XPara
{
public:
	AVCodecParameters* para = nullptr;	// ����Ƶ����
	AVRational* time_base = nullptr;	// ʱ�����
	long long total_ms = 0;				// ��ʱ�� ����
	// ��������
	static XPara* Create();
	~XPara();
private:
	// ���캯����˽�У���ֹ����ջ�ж���ֻ��new
	XPara();
};

/**
 * �̰߳�ȫavpacket list
 */
class XCODEC_API XAVPacketList
{
public:
	AVPacket* Pop();
	void Push(AVPacket* pkt);
	int Size();
	void Clear();

private:
	std::list<AVPacket*> pkts_;		// AVPacket�б�
	int max_packets_ = 1000;		// ����б���������������
	std::mutex mtx_;
};
