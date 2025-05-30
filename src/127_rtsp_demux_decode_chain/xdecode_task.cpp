#include "xdecode_task.h"
#include "xtools.h"
#include <iostream>
using namespace std;

bool XDecodeTask::Open(AVCodecParameters* para)
{
	if (!para)
	{
		LOGERROR("para is null");
		return false;
	}
	unique_lock<mutex> lock(mtx_);
	auto c = decode_.Create(para->codec_id, false);
	if (!c)
	{
		LOGERROR("decode_.Create failed!");
		return false;
	}

	// 复制视频参数
	avcodec_parameters_to_context(c, para);
	decode_.set_context(c);
	if (!decode_.Open())
	{
		LOGERROR("decode_.Open failed!");
		return false;
	}
	LOGINFO("Open decode success!");

    return true;
}

void XDecodeTask::Do(AVPacket* pkt)
{
	cout << "XDecodeTask::Do() begin" << endl;
}

void XDecodeTask::Main()
{
}
