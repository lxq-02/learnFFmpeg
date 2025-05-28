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
	 * @param stream_index ��Ӧ_ctx->streams������
	 * @param dst	�������
	 * @return �Ƿ�ɹ�
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	/**
	 * ���������ģ����������ϴ����õ�ֵ���������NULL���൱�ڹر�������
	 * 
	 * @param ctx
	 */
	void set_ctx(AVFormatContext* ctx);
	int audio_index() { return _audio_index; }	// ��ȡ��Ƶ������
	int video_index() { return _video_index; }	// ��ȡ��Ƶ������
	XRational video_time_base() { return _video_time_base; }	// ��ȡ��Ƶ��ʱ���׼
	XRational audio_time_base() { return _audio_time_base; }	// ��ȡ��Ƶ��ʱ���׼
protected:
	AVFormatContext* _ctx = nullptr;	// ��װ���װ������
	std::mutex _mtx;					// ������������_ctx�Ķ��̰߳�ȫ

	int _video_index = 0;	// video��audio��streams�е�����
	int _audio_index = 1;
	XRational _video_time_base = { 1, 25 };	// ��Ƶ��ʱ���׼��Ĭ��25fps
	XRational _audio_time_base = { 1, 44100 };	// ��Ƶ��ʱ���׼��Ĭ��44100Hz
};
