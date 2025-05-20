#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include <windows.h>
#include <mmsystem.h>  // ��Ҫ���� winmm.lib
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

		// ʣ��ʱ�� >1ms �� sleep
		if (ms - elapsed > 1) {
			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
}

int main(int argc, char* argv[])
{
	// ��߼�ʱ�����ȵ� 1ms
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
		if (chrono::duration_cast<chrono::milliseconds>(tmp - beg).count() > 1000) // ���������
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}
	timeEndPeriod(1);

	//// ����c++11��sleep
	//// ����sleep 10���� 100fps
	//auto beg = clock();		// ��ʼʱ�䣬cpu����
	//int fps = 0;			// ֡��
	//for (;;)
	//{
	//	fps++;
	//	auto tmp = clock();
	//	this_thread::sleep_for(chrono::milliseconds(10)); // 10ms
	//	//cout << clock() - tmp << " ";
	//	// 1���ӿ�ʼͳ��
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // ���������
	//	{
	//		cout << "sleep for fps: " << fps << endl;
	//		break;
	//	}
	//}

	//// ����wait�¼���ʱ���� ֡��
	//auto handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	//fps = 0;
	//beg = clock();
	//for (;;)
	//{
	//	fps++;
	//	auto tmp = clock();
	//	WaitForSingleObject(handle, 10); // 10ms
	//	cout << clock() - tmp << " ";
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // ���������
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
	//	if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // ���������
	//	{
	//		cout << "MSleep fps: " << fps << endl;
	//		break;
	//	}
	//}

	getchar();
	return 0;
}