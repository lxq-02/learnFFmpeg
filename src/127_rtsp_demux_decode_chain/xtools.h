#pragma once
#include <thread>
#include <iostream>
#include <list>
#include <mutex>
struct AVPacket;
struct AVCodecParameters;
struct AVRational;

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

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

void MSleep(unsigned int ms);

// ��ȡ��ǰʱ��� ����
long long NowMs();

/**
 * .
 */
class XThread
{
public:
	// �����߳�
	virtual void Start();

	// ֹͣ�̣߳������˳���־���ȴ��߳��˳���
	virtual void Stop();

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

protected:

	// �߳���ں���
	virtual void Main() = 0; 
	// ��־�߳��˳�
	bool is_exit_ = false;	// �õ������ж��Ƿ��˳���

	// �߳�������
	int index_ = 0;

private:
	std::thread th_;
	std::mutex mtx_;
	XThread *next_ = nullptr;	// ����������һ���ڵ�
};

class XTools
{
};

// ����Ƶ����
class XPara
{
public:
	AVCodecParameters* para = nullptr;	// ����Ƶ����
	AVRational* time_base = nullptr;	// ʱ�����
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
class XAVPacketList
{
public:
	AVPacket* Pop();
	void Push(AVPacket* pkt);
private:
	std::list<AVPacket*> pkts_;	// AVPacket�б�
	int max_packets_ = 100;		// ����б���������������
	std::mutex mtx_;
};
