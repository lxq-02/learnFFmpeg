#pragma once
#include "xformat.h"

class XDemux : public XFormat
{
public:
	/**
	 * �򿪽��װ
	 * 
	 * @param url ���װ��ַ��֧��RTSP
	 * @return	ʧ�ܷ���nullptr
	 */
	static AVFormatContext* Open(const char* url);

	/**
	 * ��ȡһ֡����
	 * 
	 * @param pkt �������
	 * @return  �Ƿ�ɹ�
	 */
	bool Read(AVPacket* pkt);
};

