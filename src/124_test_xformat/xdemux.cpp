#include "xdemux.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C" 
{
#include <libavformat/avformat.h>
}
#define BERR(err) if (err != 0) { PrintErr(err); return 0; } 
AVFormatContext* XDemux::Open(const char* url)
{
	AVFormatContext* ctx = nullptr;
	// �򿪽��װ����������
	auto re = avformat_open_input(&ctx, url,
		NULL,			// ��װ����ʽ null �Զ�̽�� ���ݺ�׺�������ļ�ͷ
		NULL			// �������ã�rtsp��Ҫ����		
	);
	BERR(re);

	// ��ȡý����Ϣ ��ͷ����ʽ
	re = avformat_find_stream_info(ctx, NULL);
	BERR(re);
	// ��ӡ��װ��Ϣ
	av_dump_format(ctx, 0, url, 0);

	return ctx;
}

bool XDemux::Read(AVPacket* pkt)
{
	unique_lock<mutex> lock(_mtx);
	if (!_ctx) return false;
	auto re = av_read_frame(_ctx, pkt);
	BERR(re);
	return true;
}
