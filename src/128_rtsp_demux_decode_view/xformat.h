#pragma once
/*****************************************************************//**
 * \file   xformat.h
 * \brief  ��װ�ͽ��װ��
 * 
 * \author lxq
 * \date   May 2025
 *********************************************************************/
#include <mutex>
#include <iostream>
#include "xtools.h"
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

using namespace std;

struct AVFormatContext;
typedef struct XRational 
{
	int num; ///< Numerator
	int den; ///< Denominator
};

void PrintErr(int err);

class XFormat
{
public:
	/**
	 * ���Ʋ������̰߳�ȫ
	 * 
	 * @param stream_index ��Ӧctx_->streams������
	 * @param dst	�������
	 * @return �Ƿ�ɹ�
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	bool CopyPara(int stream_index, AVCodecContext* dst);
	
	// ��������ָ�� ������Ƶ����
	std::shared_ptr<XPara> CopyVideoPara();
	/**
	 * ���������ģ����������ϴ����õ�ֵ���������NULL���൱�ڹر�������
	 * 
	 * @param ctx
	 */
	void set_ctx(AVFormatContext* ctx);
	int audio_index() { return audio_index_; }	// ��ȡ��Ƶ������
	int video_index() { return video_index_; }	// ��ȡ��Ƶ������
	XRational video_time_base() { return video_time_base_; }	// ��ȡ��Ƶ��ʱ���׼
	XRational audio_time_base() { return audio_time_base_; }	// ��ȡ��Ƶ��ʱ���׼

	/**
	 * ����timebase����ʱ��
	 * 
	 * @return 
	 */
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);

	int video_codec_id() const { return video_codec_id_; }	// ��ȡ��Ƶ�������ID

	/**
	 * �жϳ�ʱ
	 * 
	 * @return 
	 */
	bool IsTimeout()
	{
		if (NowMs() -last_time_ > time_out_ms_) // ��ʱ
		{
			last_time_ = NowMs();
			is_connected_ = false;
			return true;
		}
		return false;
	}

	/**
	 * ���ó�ʱʱ��
	 * 
	 * @param ms
	 */
	void set_time_out_ms(int ms);

	bool is_connected() { return is_connected_; }	// �Ƿ����ӳɹ�

protected:
	int time_out_ms_ = 0;				// ��ʱʱ�� ����
	long long last_time_ = 0;			// �ϴλ�ȡ���ݵ�ʱ�������λ����
	bool is_connected_ = false;			// �Ƿ����ӳɹ�

	AVFormatContext* ctx_ = nullptr;	// ��װ���װ������
	std::mutex mtx_;					// ������������ctx_�Ķ��̰߳�ȫ

	int video_index_ = 0;	// video��audio��streams�е�����
	int audio_index_ = 1;
	XRational video_time_base_ = { 1, 25 };	// ��Ƶ��ʱ���׼��Ĭ��25fps
	XRational audio_time_base_ = { 1, 48000 };	// ��Ƶ��ʱ���׼��Ĭ��48000Hz
	int video_codec_id_ = 0;	// �������ID
};
