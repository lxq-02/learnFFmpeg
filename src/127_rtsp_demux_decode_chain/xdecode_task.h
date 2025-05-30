#pragma once
#include "xtools.h"
#include "xdecode.h"

class XDecodeTask :public XThread
{
public:
	/**
	 * * �򿪽�����
	 * 
	 * @param para
	 * @return 
	 */
	bool Open(AVCodecParameters* para);

	// ������������
	void Do(AVPacket* pkt) override;

	// �߳�������
	void Main() override;


private:
	std::mutex mtx_;	// �̰߳�ȫ��
	XDecode decode_;
	XAVPacketList pkt_list_; // AVPacket �б����ڴ洢��������
	AVFrame* frame_ = nullptr; // �����洢
};

