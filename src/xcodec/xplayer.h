#pragma once
#include "xtools.h"
#include "xdecode_task.h"
#include "xdemux_task.h"
#include "xvideo_view.h"
#include "xaudio_play.h"

class XCODEC_API XPlayer : public XThread
{
public:
	/**
	 * 
	 * 
	 * @param url
	 * @param winid 窗口句柄
	 * @return 
	 */
	bool Open(const char* url, void* winid);

	/**
	 * 入口函数
	 * 
	 */
	void Main();

	void Do(AVPacket* pkt) override;

	/**
	 * 启动接口
	 * 
	 */
	void Start();

protected:
	XDemuxTask demux_;				// 解封装
	XDecodeTask audio_decode_;		// 音频解码
	XDecodeTask video_decode_;		// 视频解码
	XVideoView* view_ = nullptr;	// 视频渲染
};

