#pragma once
#include "xformat.h"
/**
 * 媒体封装
 */
class XCODEC_API XMux :public XFormat
{
public:
	/**
	 * 打开封装
	 * 
	 * @param url
	 * @return 
	 */
	static AVFormatContext* Open(const char* url,
		AVCodecParameters* video_para = nullptr,
		AVCodecParameters* audio_para = nullptr
	);

	bool WriteHead();
	bool Write(AVPacket* pkt);
	bool WriteEnd();

	// 音视频时间基数
	void set_src_video_time_base(AVRational* tb);
	void set_src_audio_time_base(AVRational* tb);
	~XMux();

private:
	AVRational* src_video_time_base_ = nullptr;
	AVRational* src_audio_time_base_ = nullptr;	// 源视频、音频时间基数

	long long begin_video_pts_ = -1; // 原视频开始时间
	long long begin_audio_pts_ = -1; // 原音频开始时间
};

