#include <iostream>
#include "xdemux_task.h"
#include "xmux_task.h"

using namespace std;


int main(int argc, char* argv[])
{
	cout << "test_xcodec_lib" << endl;
	XDemuxTask demux_task;

	demux_task.Open("test.mp4");
	demux_task.Start();
	getchar();
	return 0;
}
