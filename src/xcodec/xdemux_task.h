#pragma once
#include "xtools.h"
#include "xdemux.h"

class XCODEC_API XDemuxTask : public XThread
{
public:
	void Main();
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

private:
	XDemux demux_;

	std::string url_;
	int timeout_ms_ = 0; // 超时时间，单位毫秒
};

