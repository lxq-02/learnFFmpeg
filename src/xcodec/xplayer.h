#pragma once
#include "xtools.h"
#include "xdecode_task.h"
#include "xdemux_task.h"
#include "xvideo_view.h"
#include "xaudio_play.h"

class XCODEC_API XPlayer : public XThread
{

public:
	// ������Ƶ����ʼ�����ź���Ⱦ
	bool Open(const char* url, void* winid);
	// ������Դ
	void Stop();

	// ���߳� ����ͬ��
	void Main();

	// �ص���������Ƶ��
	void Do(AVPacket* pkt) override;

	// ���� ���װ ����Ƶ���� �� ����ͬ�����߳�
	void Start();

	// ��Ⱦ��Ƶ ������Ƶ
	void Update();

	void SetSpeed(float s);

	// ��ʱ�� ����
	long long total_ms() { return total_ms_; }

	// ��ǰ���ŵ�λ��
	long long pos_ms() { return pos_ms_; }

	// ������Ƶ����λ�ã�����
	bool Seek(long long ms);

	void Pause(bool is_pause) override;

protected:

	long long total_ms_ = 0;
	long long pos_ms_ = 0;
	XDemuxTask demux_;				// ���װ
	XDecodeTask audio_decode_;		// ��Ƶ����
	XDecodeTask video_decode_;		// ��Ƶ����
	XVideoView* view_ = nullptr;	// ��Ƶ��Ⱦ
};

