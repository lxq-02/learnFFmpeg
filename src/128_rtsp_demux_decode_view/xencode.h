#pragma once
#include "xcodec.h"

class XEncode : public XCodec
{
public:
	/**
	 * �������ݣ��̰߳�ȫ ÿ���´���AVPacket
	 * 
	 * @param frame �ռ����û�ά��
	 * @return ʧ�ܷ���nullptr ���ص�AVPacket�û���Ҫͨ��av_packet_free����
	 */
	AVPacket* Encode(const AVFrame* frame);

	/**
	 * �������б��뻺����AVPacket
	 * 
	 * @return 
	 */
	std::vector<AVPacket *> End();
};



