#pragma once
/*****************************************************************//**
 * \file   xformat.h
 * \brief  上下文类
 * 
 * \author lxq
 * \date   May 2025
 *********************************************************************/
#include <mutex>
#include <iostream>
#include "xtools.h"
using namespace std;

struct AVFormatContext;
typedef struct XRational 
{
	int num; ///< Numerator
	int den; ///< Denominator
};


class XCODEC_API XFormat
{
public:
	/**
	 * 复制参数，线程安全
	 * 
	 * @param stream_index 对应ctx_->streams的索引
	 * @param dst	输出参数
	 * @return 是否成功
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	bool CopyPara(int stream_index, AVCodecContext* dst);
	
	// 返回智能指针 复制视频参数
	std::shared_ptr<XPara> CopyVideoPara();
	std::shared_ptr<XPara> CopyAudioPara();
	/**
	 * 设置上下文，并且清理上次设置的值，如果传递NULL，相当于关闭上下文
	 * 
	 * @param ctx
	 */
	void set_ctx(AVFormatContext* ctx);
	int audio_index() { return audio_index_; }	// 获取音频流索引
	int video_index() { return video_index_; }	// 获取视频流索引
	XRational video_time_base() { return video_time_base_; }	// 获取视频流时间基准
	XRational audio_time_base() { return audio_time_base_; }	// 获取音频流时间基准

	/**
	 * 根据timebase计算时间
	 * 
	 * @return 
	 */
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);
	bool RescaleTime(AVPacket* pkt, long long offset_pts, AVRational* time_base);
	// 把pts dts duration值转换为毫秒
	long long RescaleToMs(long long pts, int index);

	int video_codec_id() const { return video_codec_id_; }	// 获取视频编解码器ID

	/**
	 * 判断超时
	 * 
	 * @return 
	 */
	bool IsTimeout()
	{
		if (NowMs() -last_time_ > time_out_ms_) // 超时
		{
			last_time_ = NowMs();
			is_connected_ = false;
			return true;
		}
		return false;
	}

	/**
	 * 设置超时时间
	 * 
	 * @param ms
	 */
	void set_time_out_ms(int ms);

	bool is_connected() { return is_connected_; }	// 是否连接成功

protected:
	int time_out_ms_ = 0;				// 超时时间 毫秒
	long long last_time_ = 0;			// 上次获取数据的时间戳，单位毫秒
	bool is_connected_ = false;			// 是否连接成功

	AVFormatContext* ctx_ = nullptr;	// 封装解封装上下文
	std::mutex mtx_;					// 互斥锁，保护ctx_的多线程安全

	int video_index_ = 0;	// video和audio在streams中的索引
	int audio_index_ = 1;
	XRational video_time_base_ = { 1, 25 };	// 视频流时间基准，默认25fps
	XRational audio_time_base_ = { 1, 48000 };	// 音频流时间基准，默认48000Hz
	int video_codec_id_ = 0;	// 编解码器ID
};
