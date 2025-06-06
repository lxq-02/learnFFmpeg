#pragma once
#include "xtools.h"
#include "xdecode_task.h"
#include "xdemux_task.h"
#include "xvideo_view.h"
#include "xaudio_play.h"

class XCODEC_API XPlayer : public XThread
{
public:
	/**
	 * ������Ƶ����ʼ�����ź���Ⱦ
	 * 
	 * @param url
	 * @param winid ���ھ��
	 * @return 
	 */
	bool Open(const char* url, void* winid);

	/**
	 * ������Դ
	 * 
	 */
	void Stop();

	/**
	 * ���߳� ����ͬ��
	 * 
	 */
	void Main();

	/**
	 * �ص���������Ƶ��
	 * 
	 * @param pkt
	 */
	void Do(AVPacket* pkt) override;

	/**
	 * ���� ���װ ����Ƶ���� �� ����ͬ�����߳�
	 * 
	 */
	void Start();

	/**
	 * ��Ⱦ��Ƶ ������Ƶ
	 * 
	 */
	void Update();

protected:
	XDemuxTask demux_;				// ���װ
	XDecodeTask audio_decode_;		// ��Ƶ����
	XDecodeTask video_decode_;		// ��Ƶ����
	XVideoView* view_ = nullptr;	// ��Ƶ��Ⱦ
};

