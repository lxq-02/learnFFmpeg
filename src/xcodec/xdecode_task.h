#pragma once
#include "xtools.h"
#include "xdecode.h"

class XCODEC_API XDecodeTask :public XThread
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

	// �̰߳�ȫ�����ص�ǰ��Ҫ��Ⱦ��AVFrame�����û�У�����nullptr
	// need_view_������Ⱦ
	// ���ؽ����Ҫ��XFreeFrame()�ͷ�
	AVFrame* GetFrame();

private:
	std::mutex mtx_;	// �̰߳�ȫ��
	XDecode decode_;
	XAVPacketList pkt_list_; // AVPacket �б����ڴ洢��������
	AVFrame* frame_ = nullptr; // �����洢
	bool need_view_ = false;	// �Ƿ���Ҫ��Ⱦ��ÿֻ֡��Ⱦһ�Σ�ͨ��GetFrame()��ȡ
};

