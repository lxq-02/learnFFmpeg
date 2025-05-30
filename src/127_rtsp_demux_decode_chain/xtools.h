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

// 日志级别 DEBUG INFO ERROR FATAL
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

// 获取当前时间戳 毫秒
long long NowMs();

/**
 * .
 */
class XThread
{
public:
	// 启动线程
	virtual void Start();

	// 停止线程（设置退出标志，等待线程退出）
	virtual void Stop();

	// 执行任务需要重载
	virtual void Do(AVPacket* pkt) {}

	// 传递到下一个责任链函数
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

	// 设置责任链的下一个节点（线程安全）
	void set_next(XThread* xt)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		next_ = xt;
	}

protected:

	// 线程入口函数
	virtual void Main() = 0; 
	// 标志线程退出
	bool is_exit_ = false;	// 让调用者判断是否退出了

	// 线程索引号
	int index_ = 0;

private:
	std::thread th_;
	std::mutex mtx_;
	XThread *next_ = nullptr;	// 责任链的下一个节点
};

class XTools
{
};

// 音视频参数
class XPara
{
public:
	AVCodecParameters* para = nullptr;	// 音视频参数
	AVRational* time_base = nullptr;	// 时间基数
	// 创建对象
	static XPara* Create();
	~XPara();
private:
	// 构造函数，私有，禁止创建栈中对象，只能new
	XPara();
};

/**
 * 线程安全avpacket list
 */
class XAVPacketList
{
public:
	AVPacket* Pop();
	void Push(AVPacket* pkt);
private:
	std::list<AVPacket*> pkts_;	// AVPacket列表
	int max_packets_ = 100;		// 最大列表数量，超出清理
	std::mutex mtx_;
};
