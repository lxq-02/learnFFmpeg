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
	 * 
	 * 
	 * @param url
	 * @param winid ���ھ��
	 * @return 
	 */
	bool Open(const char* url, void* winid);

	/**
	 * ��ں���
	 * 
	 */
	void Main();

	void Do(AVPacket* pkt) override;

	/**
	 * �����ӿ�
	 * 
	 */
	void Start();

protected:
	XDemuxTask demux_;				// ���װ
	XDecodeTask audio_decode_;		// ��Ƶ����
	XDecodeTask video_decode_;		// ��Ƶ����
	XVideoView* view_ = nullptr;	// ��Ƶ��Ⱦ
};

