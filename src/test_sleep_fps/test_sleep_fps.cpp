#include <iostream>
#include <ctime>
#include <thread>
#include <windows.h>
using namespace std;

int main(int argc, char* argv[])
{
	// 测试c++11的sleep
	// 测试sleep 10毫秒 100fps
	auto beg = clock();		// 开始时间，cpu跳数
	int fps = 0;			// 帧率
	for (;;)
	{
		fps++;
		auto tmp = clock();
		this_thread::sleep_for(10ms);
		cout << clock() - tmp << " ";
		// 1秒钟开始统计
		if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // 间隔毫秒数
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}

	// 测试wait事件超时控制 帧率
	auto handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	fps = 0;
	beg = clock();
	for (;;)
	{
		fps++;
		if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // 间隔毫秒数
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}

	getchar();
	return 0;
}