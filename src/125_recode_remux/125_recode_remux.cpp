#include <iostream>
#include <thread>
#include "xdemux.h"
#include "xmux.h"
#include "xdecode.h"
#include "xencode.h"
using namespace std;


#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	/////////////////////////////////////////////////////////////////////////////
	/// �����������
	string useage = "124_test_xformat �����ļ� ����ļ� ��ʼʱ�� ����ʱ�䣨�룩��Ƶ�� ��Ƶ��\n";
	useage += "124_test_xformat test.mp4 test_out.mp4 10 20 400 300";

	cout << useage << endl;
	string input_file = "";
	string out_file = "";
	if (argc < 3)
	{
		return -1;
	}
	input_file = argv[1]; // �����ļ�
	out_file = argv[2]; // ����ļ�

	/// ��ȡ10~20��֮�������Ƶ֡ ȡ�಻ȡ��
	// �ٶ�9 11���йؼ�֡������ȡ��9���
	int begin_sec = 0;	// ��ȡ��ʼʱ��
	int end_sec = 0;		// ��ȡ����ʱ��
	if (argc > 3)
		begin_sec = atoi(argv[3]); // ��ʼʱ��
	if (argc > 4)
		end_sec = atoi(argv[4]); // ����ʱ��

	int video_width = 0;
	int video_height = 0;
	if (argc > 6)
	{
		video_width = atoi(argv[5]); // ��Ƶ���
		video_height = atoi(argv[6]); // ��Ƶ�߶�
	}

	////////////////////////////////////////////////////////////////////

	//// 1����ý���ļ�
	//const char* url = "test.mp4";

	/////////////////////////////////////////////////////////////////////
	/// ���װ
	// ���װ����������
	XDemux demux; // ʹ��XDemux��
	auto demux_c = demux.Open(input_file.c_str());

	demux.set_ctx(demux_c); // ���������ĵ�XDemux����

	////////////////////////////////////////////////////////////////////////////

	
	long long video_begin_pts = 0;
	long long audio_begin_pts = 0; // ��Ƶ����pts
	long long video_end_pts = 0;

	// ��ʼ�ض����� ���������Ƶ��pts
	if (begin_sec > 0)
	{
		// ������Ƶ�Ŀ�ʼ�ͼ���ʱ��
		if (demux.video_index() >= 0 && demux.video_time_base().num > 0) // ����Ƶ��
		{
			double t = (double)demux.video_time_base().den / (double)demux.video_time_base().num; // 1���֡��
			video_begin_pts = (long long)(begin_sec * t); // ��Ƶ����pts����λΪAV_TIME_BASE
			video_end_pts = (long long)(end_sec * t); // ��Ƶ���Ľ���pts����λΪAV_TIME_BASE
			demux.Seek(video_begin_pts, demux.video_index()); // seek����Ƶ����ptsλ�� ��ʼ֡
		}

		// ������Ƶ�Ŀ�ʼ����pts
		if (demux.audio_index() >= 0 && demux.audio_time_base().num > 0) // ����Ƶ��
		{
			double t = (double)demux.audio_time_base().den / (double)demux.audio_time_base().num; // 1���֡��
			audio_begin_pts = (long long)(begin_sec * t); // ��Ƶ����pts����λΪAV_TIME_BASE
			demux.Seek(audio_begin_pts, demux.audio_index()); // seek����Ƶ����ptsλ�� ��ʼ֡
		}

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ����ĳ�ʼ��
	XDecode decode;
	auto decode_c = decode.Create(demux.video_codec_id(), false);
	// ������Ƶ����������
	demux.CopyPara(demux.video_index(), decode_c);

	decode.set_context(decode_c);
	decode.Open();
	auto frame = decode.CreateFrame(); // �����洢

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// ��Ƶ�����ʼ��
	if (demux.video_index() >= 0)
	{
		if (video_width <= 0)
			video_width = demux_c->streams[demux.video_index()]->codecpar->width;
		if (video_height <= 0)
			video_height = demux_c->streams[demux.video_index()]->codecpar->height; // ��Ƶ���
	}
	XEncode encode;
	auto encode_c = encode.Create(AV_CODEC_ID_H265, true);
	encode_c->pix_fmt = AV_PIX_FMT_YUV420P;
	encode_c->width = video_width;
	encode_c->height = video_height;
	encode.set_context(encode_c);
	encode.Open();

	////////////////////////////////////////////////////////////////////////////
/// ��װ
// ������
//const char* out_url = "test_mux.mp4";

	XMux mux; // ʹ��XMux��
	auto mux_c = mux.Open(out_file.c_str()); // �򿪷�װ���������
	mux.set_ctx(mux_c);
	auto mvs = mux_c->streams[mux.video_index()]; // ��Ƶ��
	auto mas = mux_c->streams[mux.audio_index()]; // ��Ƶ��

	// ����Ƶ
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		mvs->time_base.den = demux.video_time_base().den; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		//demux.CopyPara(demux.video_index(), mvs->codecpar); // ������Ƶ������
		// ���Ʊ�������ʽ
		avcodec_parameters_from_context(mvs->codecpar, encode_c);
	}
	// ����Ƶ
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		mas->time_base.den = demux.audio_time_base().den; // ��Ƶ��ʱ���׼����ԭ��Ƶһ��
		demux.CopyPara(demux.audio_index(), mas->codecpar); // ������Ƶ������
	}

	// д��ͷ������ı�time_base
	mux.WriteHead();

	////////////////////////////////////////////////////////////////////////////

	int audio_count = 0;
	int video_count = 0;
	double total_sec = 0;
	AVPacket pkt;
	for (;;)
	{
		if (!demux.Read(&pkt)) // ʹ��XDemux���ȡ
		{
			break; // ��ȡ����
		}

		// ��Ƶ ʱ����ڽ���ʱ��
		if (video_end_pts > 0 
			&& pkt.stream_index == demux.video_index() // ��Ƶ������
			&& pkt.pts > video_end_pts)
		{
			av_packet_unref(&pkt);
			break;
		}

		if (pkt.stream_index == demux.video_index())	// ��Ƶ
		{
			mux.RescaleTime(&pkt, video_begin_pts, demux.video_time_base());
			// ������Ƶ
			if (decode.Send(&pkt))
			{
				while (decode.Recv(frame))
				{
					cout << "." << flush;
					// �޸�ͼ��ߴ�
					// ��Ƶ����
					auto epkt = encode.Encode(frame);
					if (epkt)
					{
						// д����Ƶ֡��������pkt
						mux.Write(epkt);
						av_packet_free(&epkt);
					}
				}
			}
			video_count++;
			if (demux.video_time_base().den > 0)
				total_sec += pkt.duration * (double)demux.video_time_base().num / (double)demux.video_time_base().den; // �ۼ���Ƶʱ��
			av_packet_unref(&pkt);
		}
		else if (pkt.stream_index == demux.audio_index())
		{
			mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
			audio_count++;
			// д����Ƶ֡ ������pkt
			mux.Write(&pkt);
		}
		else
		{
			av_packet_unref(&pkt);
		}

	}

	mux.WriteEnd(); // ʹ��XMux��д��β����Ϣ
	demux.set_ctx(nullptr); // ����XDemux���е�������
	mux.set_ctx(nullptr);
	encode.set_context(nullptr); // ����XEncode���е�������  
	decode.set_context(nullptr); // ����XDecode���е�������
	cout << "����ļ���" << out_file << " : " << endl;
	cout << "��Ƶ֡��" << video_count << endl;
	cout << "��Ƶ֡��" << audio_count << endl;
	cout << "��ʱ����" << total_sec << endl;

	return 0;
}
