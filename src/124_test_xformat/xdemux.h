#pragma once
#include "xformat.h"

class XDemux : public XFormat
{
public:
	/**
	 * 打开解封装
	 * 
	 * @param url 解封装地址，支持RTSP
	 * @return	失败返回nullptr
	 */
	static AVFormatContext* Open(const char* url);

	/**
	 * 读取一帧数据
	 * 
	 * @param pkt 输出数据
	 * @return  是否成功
	 */
	bool Read(AVPacket* pkt);
};

