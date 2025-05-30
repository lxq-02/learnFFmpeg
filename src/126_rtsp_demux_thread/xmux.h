#pragma once
#include "xformat.h"
/**
 * 媒体封装
 */
class XMux :public XFormat
{
public:
	/**
	 * 打开封装
	 * 
	 * @param url
	 * @return 
	 */
	static AVFormatContext* Open(const char* url);

	bool WriteHead();
	bool Write(AVPacket* pkt);
	bool WriteEnd();
};

