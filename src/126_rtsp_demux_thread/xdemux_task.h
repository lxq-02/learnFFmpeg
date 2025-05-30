#pragma once
#include "xtools.h"
#include "xdemux.h"

class XDemuxTask : public XThread
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
private:
	XDemux demux_;

	std::string url_;
	int timeout_ms_ = 0; // ��ʱʱ�䣬��λ����
};

