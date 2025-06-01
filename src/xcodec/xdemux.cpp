#include "xdemux.h"
#include "xtools.h"
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

	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); // ʹ��ý����ΪtcpЭ�飬Ĭ��Ϊudp
	//av_dict_set(&opts, "stimeout", "1000000", 0); // ���ӳ�ʱ1��

	auto re = avformat_open_input(&ctx, url,
		NULL,			// ��װ����ʽ null �Զ�̽�� ���ݺ�׺�������ļ�ͷ
		&opts			// �������ã�rtsp��Ҫ����		
	);
	if (opts)
		av_dict_free(&opts);
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
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_read_frame(ctx_, pkt);
	BERR(re);
	// ��ʱ�����ڳ�ʱ�ж�
	last_time_ = NowMs();
	return true;
}

bool XDemux::Seek(long long pts, int stream_index)
{
	unique_lock<mutex> lock(mtx_);
	if (!ctx_) return false;
	auto re = av_seek_frame(ctx_, stream_index, pts, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	BERR(re);
	return true;
}
