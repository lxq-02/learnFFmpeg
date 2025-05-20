#include <iostream>
#include <ctime>
#include <thread>
#include <windows.h>
using namespace std;

int main(int argc, char* argv[])
{
	// ����c++11��sleep
	// ����sleep 10���� 100fps
	auto beg = clock();		// ��ʼʱ�䣬cpu����
	int fps = 0;			// ֡��
	for (;;)
	{
		fps++;
		auto tmp = clock();
		this_thread::sleep_for(10ms);
		cout << clock() - tmp << " ";
		// 1���ӿ�ʼͳ��
		if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // ���������
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}

	// ����wait�¼���ʱ���� ֡��
	auto handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	fps = 0;
	beg = clock();
	for (;;)
	{
		fps++;
		if ((clock() - beg) / (CLOCKS_PER_SEC / 1000) > 1000) // ���������
		{
			cout << "sleep for fps: " << fps << endl;
			break;
		}
	}

	getchar();
	return 0;
}