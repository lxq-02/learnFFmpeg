#include <iostream>
#include <thread>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")


void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}
#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	// 1����ý���ļ�
	const char* url = "test.mp4";
	// ���װ����������
	AVFormatContext* ic = nullptr;
	auto re = avformat_open_input(&ic, url,
		NULL,			// ��װ����ʽ null �Զ�̽�� ���ݺ�׺�������ļ�ͷ
		NULL			// �������ã�rtsp��Ҫ����		
	);
	CERR(re);

	// 2����ȡý����Ϣ ��ͷ����ʽ
	re = avformat_find_stream_info(ic, NULL);
	CERR(re);

	// ��ӡ��װ��Ϣ
	av_dump_format(ic, 0, url,
		0 // 0��ʾ������������ 1���
	);

	AVStream* as = nullptr; // ��Ƶ��
	AVStream* vs = nullptr; // ��Ƶ��
	for (int i = 0; i < ic->nb_streams; ++i)
	{
		// ��Ƶ
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			as = ic->streams[i];
			cout << "=============��Ƶ===============" << endl;
			cout << "sample_rate:" << as->codecpar->sample_rate << endl;
		}
		else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vs = ic->streams[i];
			cout << "=============��Ƶ===============" << endl;
			cout << "width:" << vs->codecpar->width << endl;
			cout << "height:" << vs->codecpar->height << endl;
		}
	}

	AVPacket pkt;
	for (;;)
	{
		re = av_read_frame(ic, &pkt);
		CERR(re);
		if (vs && pkt.stream_index == vs->index)
		{
			cout << "��Ƶ��";
		}
		else if (as && pkt.stream_index == as->index)
		{
			cout << "��Ƶ��";
		}
		cout << pkt.pts << " : " << pkt.dts << " : " << pkt.size << endl;
		// 
		av_packet_unref(&pkt);
		this_thread::sleep_for(chrono::milliseconds(100));
	}


	avformat_close_input(&ic);

	return 0;
}
