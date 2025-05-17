#pragma once

#include <SDL.h>

class CondMutex
{
public:
	CondMutex();
	~CondMutex();

	void lock();
	void unlock();
	void signal();		// 通知等待的一个线程继续执行（类似于单播）
	void broadcast();	// 通知所有等待的线程继续执行（类似于广播）
	void wait();		// 当前线程等待条件变量的信号，等待时会释放锁，直到接收到信号后重新获取锁

private:
	// 互斥锁
	SDL_mutex* _mutex = nullptr;
	// 条件变量
	SDL_cond* _cond = nullptr;
};

