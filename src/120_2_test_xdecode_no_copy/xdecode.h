#pragma once
#include "xcodec.h"
class XDecode :public XCodec
{
public:
	bool Send(const AVPacket* pkt); // ���ͽ���
	bool Recv(AVFrame* frame, bool is_hw_copy = true);		// ��ȡ����
	std::vector<AVFrame*> End();	// ��ȡ��������

	/**
	 * ��ʼ��Ӳ������ 
	 * 
	 * @param type 4 AV_HWDEVICE_TYPE_DXVA2
	 * @return 
	 */
	bool InitHW(int type = 4);
};

