#include <iostream>
using namespace std;

extern "C" // 指令函数是C语言函数，函数名不包含重载标注
{
// 引用ffmpeg头文件
#include <libavcodec/avcodec.h>
}

// 预处理指令导入库
#pragma comment(lib, "avcodec.lib")

int main(int argc, char* argv[])
{
	cout << "first ffmpeg " << endl;
	cout << avcodec_configuration() << endl; // 打印ffmpeg版本信息
	return 0;
}