#pragma once

#include <SDL.h>

class CondMutex
{
public:
	CondMutex();
	~CondMutex();

	void lock();
	void unlock();
	void signal();		// ֪ͨ�ȴ���һ���̼߳���ִ�У������ڵ�����
	void broadcast();	// ֪ͨ���еȴ����̼߳���ִ�У������ڹ㲥��
	void wait();		// ��ǰ�̵߳ȴ������������źţ��ȴ�ʱ���ͷ�����ֱ�����յ��źź����»�ȡ��

private:
	// ������
	SDL_mutex* _mutex = nullptr;
	// ��������
	SDL_cond* _cond = nullptr;
};

