#include "xtools.h"
#include <sstream>
#include <chrono>
using namespace std;

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
	if (th_.joinable()) // 判断子线程是否可以等待
	{
		th_.join();		// 等待子线程退出
	}
	ss.str("");
	ss << "XThread::Stop() " << index_;
	LOGINFO(ss.str());


}
