#include <iostream>
#include <thread>
#include "xdemux.h"
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


	//AVFormatContext* ic = nullptr;
	//auto re = avformat_open_input(&ic, url,
	//	NULL,			// ��װ����ʽ null �Զ�̽�� ���ݺ�׺�������ļ�ͷ
	//	NULL			// �������ã�rtsp��Ҫ����		
	//);
	//CERR(re);

	//// 2����ȡý����Ϣ ��ͷ����ʽ
	//re = avformat_find_stream_info(ic, NULL);
	//CERR(re);

	//// ��ӡ��װ��Ϣ
	//av_dump_format(ic, 0, url,
	//	0 // 0��ʾ������������ 1���
	//);

	//AVStream* as = nullptr; // ��Ƶ��
	//AVStream* vs = nullptr; // ��Ƶ��
	//for (int i = 0; i < ic->nb_streams; ++i)
	//{
	//	// ��Ƶ
	//	if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	//	{
	//		as = ic->streams[i];
	//		cout << "=============��Ƶ===============" << endl;
	//		cout << "sample_rate:" << as->codecpar->sample_rate << endl;
	//	}
	//	else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	//	{
	//		vs = ic->streams[i];
	//		cout << "=============��Ƶ===============" << endl;
	//		cout << "width:" << vs->codecpar->width << endl;
	//		cout << "height:" << vs->codecpar->height << endl;
	//	}
	//}
	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// ��װ
	// ������
	const char* out_url = "test_out.mp4";
	AVFormatContext* ec = nullptr;
	auto re = avformat_alloc_output_context2(&ec, NULL, NULL, 
		out_url // �����ļ����Ʋ��װ��ʽ
	);
	CERR(re);
	// �����Ƶ������Ƶ��
	auto mvs = avformat_new_stream(ec, NULL);	// ��Ƶ��
	auto mas = avformat_new_stream(ec, NULL);	// ��Ƶ��

	// �����IO
	re = avio_open(&ec->pb, out_url, AVIO_FLAG_WRITE);
	CERR(re);

	// ���ñ�������Ƶ������
	//ec->streams[0]��
	//mvs->codecpar;// ��Ƶ������
	//if (vs)
	//{
	//	mvs->time_base = vs->time_base; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
	//	// �ӽ��װ���Ʋ���
	//	avcodec_parameters_copy(mvs->codecpar, vs->codecpar);
	//}
	mvs->time_base.num = demux.video_time_base().num;
	mvs->time_base.den = demux.video_time_base().den; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��

	mas->time_base.num = demux.audio_time_base().num;
	mas->time_base.den = demux.audio_time_base().den; 

	demux.CopyPara(demux.video_index(), mvs->codecpar); // ������Ƶ��������XDemux����
	demux.CopyPara(demux.audio_index(), mas->codecpar); // ������Ƶ��������XDemux����
	//if (as)
	//{
	//	mas->time_base = as->time_base; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
	//	// �ӽ��װ���Ʋ���
	//	avcodec_parameters_copy(mas->codecpar, as->codecpar);
	//}

	// д��ͷ����Ϣ
	re = avformat_write_header(ec, NULL);
	CERR(re);

	av_dump_format(ec, 0, out_url, 1); // ��ӡ��װ��Ϣ
	
	
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
		//pkt.pos = -1; // ����posΪ-1����ʾδ֪λ��




		// д������Ƶ֡ ������pkt
		re = av_interleaved_write_frame(ec, &pkt);
		if (re != 0)
		{
			PrintErr(re);
		}
		//av_packet_unref(&pkt);
		//this_thread::sleep_for(chrono::milliseconds(100));
	}

	// д��β����Ϣ �����ļ�ƫ��
	re = av_write_trailer(ec);
	if (re != 0)
	{
		PrintErr(re);
	}

	//avformat_close_input(&ic);
	demux.set_ctx(nullptr); // ����XDemux���е�������

	avio_closep(&ec->pb);
	avformat_free_context(ec);
	ec = nullptr;
	return 0;
}
