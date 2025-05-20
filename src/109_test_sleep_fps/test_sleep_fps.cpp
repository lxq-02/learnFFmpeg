#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include <windows.h>
#include <mmsystem.h>  // 需要链接 winmm.lib
#pragma comment(lib, "winmm.lib")
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
		if (ms - elapsed > 1) {
			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
}

int main(int argc, char* argv[])
{
	// 提高计时器精度到 1ms
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(1);

	auto beg = chrono::high_resolution_clock::now();
	int fps = 0;
	for (;;)
	{
		fps++;
		MSleep(10);
		auto tmp = chrono::high_resolution_clock::now();
		if (chrono::duration_cast<chrono::milliseconds>(tmp - beg).count() > 1000) // 间隔毫秒数
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}
	timeEndPeriod(1);

	//// 测试c++11的sleep
	//// 测试sleep 10毫秒 100fps
	//auto beg = clock();		// 开始时间，cpu跳数
	//int fps = 0;			// 帧率
	//for (;;)
	//{
	//	fps++;
	//	auto tmp = clock();
	//	this_thread::sleep_for(chrono::milliseconds(10)); // 10ms
	//	//cout << clock() - tmp << " ";
	//	// 1秒钟开始统计
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // 间隔毫秒数
	//	{
	//		cout << "sleep for fps: " << fps << endl;
	//		break;
	//	}
	//}

	//// 测试wait事件超时控制 帧率
	//auto handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	//fps = 0;
	//beg = clock();
	//for (;;)
	//{
	//	fps++;
	//	auto tmp = clock();
	//	WaitForSingleObject(handle, 10); // 10ms
	//	cout << clock() - tmp << " ";
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // 间隔毫秒数
	//	{
	//		cout << "wait fps: " << fps << endl;
	//		break;
	//	}
	//}

	//fps = 0;
	//beg = clock();
	//for (;;)
	//{
	//	fps++;
	//	auto tmp = clock();
	//	MSleep(10);
	//	cout << clock() - tmp << " ";
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // 间隔毫秒数
	//	{
	//		cout << "MSleep fps: " << fps << endl;
	//		break;
	//	}
	//}

	getchar();
	return 0;
}