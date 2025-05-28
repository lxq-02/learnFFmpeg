#include <iostream>
#include <thread>
#include "xdemux.h"
using namespace std;


#define CERR(err) if( err != 0 ){ PrintErr(err); getchar(); return -1;}

int main(int argc, char* argv[])
{
	// 1、打开媒体文件
	const char* url = "test.mp4";

	/////////////////////////////////////////////////////////////////////
	/// 解封装
	// 解封装输入上下文
	XDemux demux; // 使用XDemux类
	auto demux_c = demux.Open(url);

	demux.set_ctx(demux_c); // 设置上下文到XDemux类中


	//AVFormatContext* ic = nullptr;
	//auto re = avformat_open_input(&ic, url,
	//	NULL,			// 封装器格式 null 自动探测 根据后缀名或者文件头
	//	NULL			// 参数设置，rtsp需要设置		
	//);
	//CERR(re);

	//// 2、获取媒体信息 无头部格式
	//re = avformat_find_stream_info(ic, NULL);
	//CERR(re);

	//// 打印封装信息
	//av_dump_format(ic, 0, url,
	//	0 // 0表示上下文是输入 1输出
	//);

	//AVStream* as = nullptr; // 音频流
	//AVStream* vs = nullptr; // 视频流
	//for (int i = 0; i < ic->nb_streams; ++i)
	//{
	//	// 音频
	//	if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	//	{
	//		as = ic->streams[i];
	//		cout << "=============音频===============" << endl;
	//		cout << "sample_rate:" << as->codecpar->sample_rate << endl;
	//	}
	//	else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	//	{
	//		vs = ic->streams[i];
	//		cout << "=============视频===============" << endl;
	//		cout << "width:" << vs->codecpar->width << endl;
	//		cout << "height:" << vs->codecpar->height << endl;
	//	}
	//}
	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// 封装
	// 上下文
	const char* out_url = "test_out.mp4";
	AVFormatContext* ec = nullptr;
	auto re = avformat_alloc_output_context2(&ec, NULL, NULL, 
		out_url // 根据文件名推测封装格式
	);
	CERR(re);
	// 添加视频流、音频流
	auto mvs = avformat_new_stream(ec, NULL);	// 视频流
	auto mas = avformat_new_stream(ec, NULL);	// 音频流

	// 打开输出IO
	re = avio_open(&ec->pb, out_url, AVIO_FLAG_WRITE);
	CERR(re);

	// 设置编码音视频流参数
	//ec->streams[0]；
	//mvs->codecpar;// 视频流参数
	//if (vs)
	//{
	//	mvs->time_base = vs->time_base; // 视频流时间基准，与原视频一致
	//	// 从解封装复制参数
	//	avcodec_parameters_copy(mvs->codecpar, vs->codecpar);
	//}
	mvs->time_base.num = demux.video_time_base().num;
	mvs->time_base.den = demux.video_time_base().den; // 视频流时间基准，与原视频一致

	mas->time_base.num = demux.audio_time_base().num;
	mas->time_base.den = demux.audio_time_base().den; 

	demux.CopyPara(demux.video_index(), mvs->codecpar); // 复制视频流参数到XDemux类中
	demux.CopyPara(demux.audio_index(), mas->codecpar); // 复制音频流参数到XDemux类中
	//if (as)
	//{
	//	mas->time_base = as->time_base; // 音频流时间基准，与原音频一致
	//	// 从解封装复制参数
	//	avcodec_parameters_copy(mas->codecpar, as->codecpar);
	//}

	// 写入头部信息
	re = avformat_write_header(ec, NULL);
	CERR(re);

	av_dump_format(ec, 0, out_url, 1); // 打印封装信息
	
	
	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	/// 截取10~20秒之间的音视频帧 取多不取少
	// 假定9 11秒有关键帧，我们取第9秒的
	double begin_sec = 10.0;	// 截取开始时间
	double end_sec = 20.0;		// 截取结束时间
	long long begin_pts = 0;
	long long begin_audio_pts = 0; // 音频流的pts
	long long end_pts = 0;
	// 计算pts 换算成输入ic的pts，以视频流为准
	//if (vs && vs->time_base.num > 0)
	//{
	//	// sec/time_base = pts
	//	// pts = sec / (num/den) = sec * (den/num)
	//	double t = (double)vs->time_base.den / (double)vs->time_base.num; // 分母/分子 = 1秒的帧数
	//	begin_pts = begin_sec * t;
	//	end_pts = end_sec * t;
	//}
	//if (as && as->time_base.num > 0)
	//	begin_audio_pts = begin_pts * ((double)as->time_base.den / (double)as->time_base.num); // 音频流的pts与视频流一致

	// seek输入媒体文件 移动到第十秒的关键帧位置
	//if (vs)
	//	re = av_seek_frame(ic, vs->index, begin_pts, 
	//		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME); // 向后关键帧
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
		if (!demux.Read(&pkt)) // 使用XDemux类读取
		{
			break; // 读取结束
		}
		//AVStream* in_stream = ic->streams[pkt.stream_index];
		//AVStream* out_stream = nullptr;
		//long long offset_pts = 0; // 偏移pts，用于截断的开头pts运算
		//if (vs && pkt.stream_index == vs->index)
		//{
		//	cout << "视频：";

		//	// 超过第20秒退出，只存10~20秒
		//	if (pkt.pts > end_pts)
		//	{
		//		av_packet_unref(&pkt); // 清理包
		//		break; // 退出循环
		//	}
		//	out_stream = ec->streams[0]; // 视频流
		//	offset_pts = begin_pts; // 视频流的pts偏移
		//}
		//else if (as && pkt.stream_index == as->index)
		//{
		//	cout << "音频：";
		//	out_stream = ec->streams[1]; // 音频流
		//	offset_pts = begin_audio_pts;
		//	// 增加音频停止判断
		//	if (pkt.pts > begin_audio_pts + (end_pts - begin_pts)) {
		//		av_packet_unref(&pkt);
		//		continue; // 不要 break，跳过这帧
		//	}
		//}
		//cout << pkt.pts << " : " << pkt.dts << " : " << pkt.size << endl;

		//// 重新计算pts dts duration
		//// a * bq (输入basetime）/ cq（输出basetime） = a * bq / cq
		//pkt.pts = av_rescale_q_rnd(pkt.pts - offset_pts, in_stream->time_base,
		//	out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.dts = av_rescale_q_rnd(pkt.dts - offset_pts, in_stream->time_base,
		//	out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base,
		//	out_stream->time_base);
		//pkt.pos = -1; // 重置pos为-1，表示未知位置




		// 写入音视频帧 会清理pkt
		re = av_interleaved_write_frame(ec, &pkt);
		if (re != 0)
		{
			PrintErr(re);
		}
		//av_packet_unref(&pkt);
		//this_thread::sleep_for(chrono::milliseconds(100));
	}

	// 写入尾部信息 包含文件偏移
	re = av_write_trailer(ec);
	if (re != 0)
	{
		PrintErr(re);
	}

	//avformat_close_input(&ic);
	demux.set_ctx(nullptr); // 清理XDemux类中的上下文

	avio_closep(&ec->pb);
	avformat_free_context(ec);
	ec = nullptr;
	return 0;
}
