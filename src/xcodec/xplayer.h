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
	 * 打开音视频，初始化播放和渲染
	 * 
	 * @param url
	 * @param winid 窗口句柄
	 * @return 
	 */
	bool Open(const char* url, void* winid);

	/**
	 * 清理资源
	 * 
	 */
	void Stop();

	/**
	 * 主线程 处理同步
	 * 
	 */
	void Main();

	/**
	 * 回调接收音视频包
	 * 
	 * @param pkt
	 */
	void Do(AVPacket* pkt) override;

	/**
	 * 开启 解封装 音视频解码 和 处理同步的线程
	 * 
	 */
	void Start();

	/**
	 * 渲染视频 播放音频
	 * 
	 */
	void Update();

protected:
	XDemuxTask demux_;				// 解封装
	XDecodeTask audio_decode_;		// 音频解码
	XDecodeTask video_decode_;		// 视频解码
	XVideoView* view_ = nullptr;	// 视频渲染
};

