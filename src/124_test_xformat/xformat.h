#pragma once
/*****************************************************************//**
 * \file   xformat.h
 * \brief  封装和解封装类
 * 
 * \author lxq
 * \date   May 2025
 *********************************************************************/
#include <mutex>
#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

using namespace std;

struct AVFormatContext;
typedef struct XRational 
{
	int num; ///< Numerator
	int den; ///< Denominator
};

void PrintErr(int err);

class XFormat
{
public:
	/**
	 * 复制参数，线程安全
	 * 
	 * @param stream_index 对应_ctx->streams的索引
	 * @param dst	输出参数
	 * @return 是否成功
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	/**
	 * 设置上下文，并且清理上次设置的值，如果传递NULL，相当于关闭上下文
	 * 
	 * @param ctx
	 */
	void set_ctx(AVFormatContext* ctx);
	int audio_index() { return _audio_index; }	// 获取音频流索引
	int video_index() { return _video_index; }	// 获取视频流索引
	XRational video_time_base() { return _video_time_base; }	// 获取视频流时间基准
	XRational audio_time_base() { return _audio_time_base; }	// 获取音频流时间基准
protected:
	AVFormatContext* _ctx = nullptr;	// 封装解封装上下文
	std::mutex _mtx;					// 互斥锁，保护_ctx的多线程安全

	int _video_index = 0;	// video和audio在streams中的索引
	int _audio_index = 1;
	XRational _video_time_base = { 1, 25 };	// 视频流时间基准，默认25fps
	XRational _audio_time_base = { 1, 44100 };	// 音频流时间基准，默认44100Hz
};
