#include <iostream>
using namespace std;

extern "C" // ָ�����C���Ժ��������������������ر�ע
{
// ����ffmpegͷ�ļ�
#include <libavcodec/avcodec.h>
}

// Ԥ����ָ����
#pragma comment(lib, "avcodec.lib")

int main(int argc, char* argv[])
{
	cout << "first ffmpeg " << endl;
	cout << avcodec_configuration() << endl; // ��ӡffmpeg�汾��Ϣ
	return 0;
}