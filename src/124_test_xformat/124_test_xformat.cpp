#include <iostream>
#include <thread>
#include "xdemux.h"
#include "xmux.h"
using namespace std;


#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	// 1����ý���ļ�
	const char* url = "test.mp4";

	/////////////////////////////////////////////////////////////////////
	/// ���װ
	// ���װ����������
	XDemux demux; // ʹ��XDemux��
	auto demux_c = demux.Open(url);

	demux.set_ctx(demux_c); // ���������ĵ�XDemux����

	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// ��װ
	// ������
	const char* out_url = "test_mux.mp4";

	XMux mux; // ʹ��XMux��
	auto mux_c = mux.Open(out_url); // �򿪷�װ���������
	mux.set_ctx(mux_c);
	auto mvs = mux_c->streams[mux.video_index()]; // ��Ƶ��
	auto mas = mux_c->streams[mux.audio_index()]; // ��Ƶ��

	// ����Ƶ
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		mvs->time_base.den = demux.video_time_base().den; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		demux.CopyPara(demux.video_index(), mvs->codecpar); // ������Ƶ������
	}
	// ����Ƶ
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		mas->time_base.den = demux.audio_time_base().den; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		demux.CopyPara(demux.audio_index(), mas->codecpar); // ������Ƶ������
	}

	mux.WriteHead();

	//// д��ͷ����Ϣ
	//re = avformat_write_header(ec, NULL);
	//CERR(re);

	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// ��ȡ10~20��֮�������Ƶ֡ ȡ�಻ȡ��
	// �ٶ�9 11���йؼ�֡������ȡ��9���
	double begin_sec = 10.0;	// ��ȡ��ʼʱ��
	double end_sec = 20.0;		// ��ȡ����ʱ��
	long long begin_pts = 0;
	long long begin_audio_pts = 0; // ��Ƶ����pts
	long long end_pts = 0;
	// ����pts ���������ic��pts������Ƶ��Ϊ׼
	//if (vs && vs->time_base.num > 0)
	//{
	//	// sec/time_base = pts
	//	// pts = sec / (num/den) = sec * (den/num)
	//	double t = (double)vs->time_base.den / (double)vs->time_base.num; // ��ĸ/���� = 1���֡��
	//	begin_pts = begin_sec * t;
	//	end_pts = end_sec * t;
	//}
	//if (as && as->time_base.num > 0)
	//	begin_audio_pts = begin_pts * ((double)as->time_base.den / (double)as->time_base.num); // ��Ƶ����pts����Ƶ��һ��

	// seek����ý���ļ� �ƶ�����ʮ��Ĺؼ�֡λ��
	//if (vs)
	//	re = av_seek_frame(ic, vs->index, begin_pts, 
	//		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME); // ���ؼ�֡
	//CERR(re);



	AVPacket pkt;
	for (;;)
	{
		//re = av_read_frame(ic, &pkt);
		//if (re != 0)
		//{
		//	PrintErr(re);
		//	break;
		//}
		if (!demux.Read(&pkt)) // ʹ��XDemux���ȡ
		{
			break; // ��ȡ����
		}
		//AVStream* in_stream = ic->streams[pkt.stream_index];
		//AVStream* out_stream = nullptr;
		//long long offset_pts = 0; // ƫ��pts�����ڽضϵĿ�ͷpts����
		//if (vs && pkt.stream_index == vs->index)
		//{
		//	cout << "��Ƶ��";

		//	// ������20���˳���ֻ��10~20��
		//	if (pkt.pts > end_pts)
		//	{
		//		av_packet_unref(&pkt); // �����
		//		break; // �˳�ѭ��
		//	}
		//	out_stream = ec->streams[0]; // ��Ƶ��
		//	offset_pts = begin_pts; // ��Ƶ����ptsƫ��
		//}
		//else if (as && pkt.stream_index == as->index)
		//{
		//	cout << "��Ƶ��";
		//	out_stream = ec->streams[1]; // ��Ƶ��
		//	offset_pts = begin_audio_pts;
		//	// ������Ƶֹͣ�ж�
		//	if (pkt.pts > begin_audio_pts + (end_pts - begin_pts)) {
		//		av_packet_unref(&pkt);
		//		continue; // ��Ҫ break��������֡
		//	}
		//}
		//cout << pkt.pts << " : " << pkt.dts << " : " << pkt.size << endl;

		//// ���¼���pts dts duration
		//// a * bq (����basetime��/ cq�����basetime�� = a * bq / cq
		//pkt.pts = av_rescale_q_rnd(pkt.pts - offset_pts, in_stream->time_base,
		//	out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.dts = av_rescale_q_rnd(pkt.dts - offset_pts, in_stream->time_base,
		//	out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base,
		//	out_stream->time_base);
		pkt.pos = -1; // ����posΪ-1����ʾδ֪λ��




		// д������Ƶ֡ ������pkt
		mux.Write(&pkt); // ʹ��XMux��д��
		//av_packet_unref(&pkt);
		//this_thread::sleep_for(chrono::milliseconds(100));
	}

	//// д��β����Ϣ �����ļ�ƫ��
	//re = av_write_trailer(ec);
	//if (re != 0)
	//{
	//	PrintErr(re);
	//}
	mux.WriteEnd(); // ʹ��XMux��д��β����Ϣ

	//avformat_close_input(&ic);
	demux.set_ctx(nullptr); // ����XDemux���е�������
	mux.set_ctx(nullptr);

	//avio_closep(&ec->pb);
	//avformat_free_context(ec);
	//ec = nullptr;
	return 0;
}
