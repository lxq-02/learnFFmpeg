#pragma once
#include "xtools.h"
#include "xdemux.h"

enum XSYN_TYPE
{
	XSYN_NONE = 0, // ����ͬ��
	XSYN_VIDEO = 1 // ������Ƶͬ������������Ƶ
};

class XCODEC_API XDemuxTask : public XThread
{
public:
	void Main();
	/**
	 * �򿪽��װ
	 * 
	 * @param url rtsp��ַ
	 * @param timeout_ms ��ʱʱ�䣬��λ����
	 * @return 
	 */
	bool Open(std::string url, int timeout_ms = 1000);

	// ������Ƶ����
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
	int timeout_ms_ = 0; // ��ʱʱ�䣬��λ����
	XSYN_TYPE syn_type_ = XSYN_NONE;
};

