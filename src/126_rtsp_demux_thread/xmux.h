#pragma once
#include "xformat.h"
/**
 * ý���װ
 */
class XMux :public XFormat
{
public:
	/**
	 * �򿪷�װ
	 * 
	 * @param url
	 * @return 
	 */
	static AVFormatContext* Open(const char* url);

	bool WriteHead();
	bool Write(AVPacket* pkt);
	bool WriteEnd();
};

