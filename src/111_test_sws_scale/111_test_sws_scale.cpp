#include <iostream>
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avcodec.lib")

int main(int argc, char* argv[])
{
	cout << "111_test_sws_scale" << endl;
	return 0;
}