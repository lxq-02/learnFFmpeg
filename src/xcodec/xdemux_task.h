#pragma once
#include "xtools.h"
#include "xdemux.h"

enum XSYN_TYPE
{
	XSYN_NONE = 0, // 不做同步
	XSYN_VIDEO = 1 // 根据视频同步，不处理音频
};

class XCODEC_API XDemuxTask : public XThread
{
public:
	// 控制进度
	bool Seek(long long ms);

	int audio_index() { return demux_.audio_index(); }	// 获取音频流索引
	int video_index() { return demux_.video_index(); }	// 获取视频流索引

	void Main();

	/**
	 *	资源清理
	 *
	 */
	void Stop();

	/**
	 * 打开解封装
	 * 
	 * @param url rtsp地址
	 * @param timeout_ms 超时时间，单位毫秒
	 * @return 
	 */
	bool Open(std::string url, int timeout_ms = 1000);

	// 复制视频参数
	std::shared_ptr<XPara>  CopyVideoPara()
	{
		return demux_.CopyVideoPara();
	}

	std::shared_ptr<XPara> CopyAudioPara()
	{
		return demux_.CopyAudioPara();
	}

	void set_syn_type(XSYN_TYPE type) {	syn_type_ = type; }

private:
	XDemux demux_;

	std::string url_;
	int timeout_ms_ = 0; // 超时时间，单位毫秒
	XSYN_TYPE syn_type_ = XSYN_NONE;
};

