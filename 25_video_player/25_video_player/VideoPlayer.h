#pragma once

#include <QObject>
#include <list>
#include "condmutex.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#define ERROR_BUF \
	char errbuf[1024]; \
	av_strerror(ret, errbuf, sizeof (errbuf));

#define CODE(func, code) \
	if (ret < 0) \
	{ \
		ERROR_BUF; \
		qDebug() << #func << "error" << errbuf; \
		code; \
	}

#define END(func) CODE(func, fataError(); return;)
#define RET(func) CODE(func, return ret;)
#define CONTINUE(func) CODE(func, continue);
#define BREAK(func) CODE(func, break;)

// Ԥ������Ƶ���ݣ���������ʾ����Ⱦ��Ƶ��
class VideoPlayer : public QObject
{
	Q_OBJECT
public:
	// ״̬
	typedef enum
	{
		Stopped = 0,
		Playing,
		Paused
	}State;

	// ����
	typedef enum
	{
		Min = 0,
		Max = 100
	}Volumn;

	// ��Ƶframe����
	typedef struct
	{
		int width;
		int height;
		AVPixelFormat pixFmt;	// ���ظ�ʽ
		int size;				// ��Ƶ֡���ݴ�С
	}VideoSwsSpec;

	explicit VideoPlayer(QObject* parent = nullptr);
	~VideoPlayer();

	// ����
	void play();
	// ��ͣ
	void pause();
	// ֹͣ
	void stop();
	// �Ƿ����ڲ�����
	bool isPlaying();
	// ��ȡ��ǰ��״̬
	State getState();
	// �����ļ���
	void setFilename(QString& filename);
	// ��ȡ��ʱ������λ��΢�룬1��=10^3����=10^6΢�
	// ��ȡ��ʱ������λ���룩
	int getDuration();
	// ��ǰ�Ĳ���ʱ�̣���λ���룩
	int getTime();
	// ���õ�ǰ�Ĳ���ʱ�̣���λ���룩
	void setTime(int seekTime);
	// ��������
	void setVolumn(int volumn);
	int getVolumn();
	// ���þ���
	void setMute(bool mute);
	bool isMute();

signals:
	void stateChanged(VideoPlayer* player);
	void timeChanged(VideoPlayer* player);
	void initFinished(VideoPlayer* player);
	void playFailed(VideoPlayer* player);
	void frameDecoded(VideoPlayer* player,
						uint8_t* data,
						VideoSwsSpec& spec);

private:
	/******** ��Ƶ��� *********/
	typedef struct
	{
		int sampleRate;
		AVSampleFormat sampleFmt;
		int chLayout;
		int chs;
		int bytesPerSampleFrame;
	}AudioSwrSpec;

	// ����������
	AVCodecContext* _aDecodecCtx = nullptr;
	// ��
	AVStream* _aStream = nullptr;
	// �����Ƶ�����б�
	std::list<AVPacket> _aPktList;
	// ��Ƶ���б����
	CondMutex _aMutex;
	// ��Ƶ�ز���������
	SwrContext* _aSwrCtx = nullptr;
	// ��Ƶ�ز������롢�������
	AudioSwrSpec _aSwrInSpec, _aSwrOutSpec;
	// ��Ƶ�ز������롢���frame
	AVFrame* _aSwrInFrame = nullptr, * _aSwrOutFrame = nullptr;
	// ��Ƶ�ز������PCM�����������ĸ�λ�ÿ�ʼȡ��PCM������䵽SDL����Ƶ��������
	int _aSwrOutIdx = 0;
	// ��Ƶ�ز������PCM�Ĵ�С
	int _aSwrOutSize = 0;
	// ��Ƶʱ�ӣ���ǰ��Ƶ����Ӧ��ʱ��ֵ
	double _aTime = 0;
	// ��Ƶ��Դ�Ƿ�����ͷ�
	bool _aCanFree = false;
	// �������õĵ�ǰ����ʱ�̣��������seek���ܣ�
	int _aSeekTime = -1;
	// �Ƿ�����Ƶ��
	bool _hasAudio = false;

	// ��ʼ����Ƶ��Ϣ
	int initAudioInfo();
	// ��ʼ����Ƶ�ز���
	int initSwr();
	// ��ʼ��SDL
	int initSDL();
	// ������ݰ�����Ƶ���б���
	void addAudioPkt(AVPacket& pkt);
	// �����Ƶ���б�
	void clearAudioPktList();
	// SDL��仺�����Ļص�����
	static void sdlAudioCallbackFunc(void* userdata, Uint8* stream, int len);
	// SDL��仺�����Ļص�����
	void sdlAudioCallback(Uint8* stream, int len);
	// ��Ƶ����
	int decodeAudio();

	/********** ��Ƶ��� **********/
	// ����������
	AVCodecContext* _vDecodeCtx = nullptr;
	// ��
	AVStream* _vStream = nullptr;
	// ���ظ�ʽת�������롢���frame
	AVFrame* _vSwsInFrame = nullptr, * _vSwsOutFrame = nullptr;
	// ���ظ�ʽת����������
	SwsContext* _vSwsCtx = nullptr;
	// ���ظ�ʽת�������frame�Ĳ���
	VideoSwsSpec _vSwsOutSpec;
	// �����Ƶ�����б�
	std::list<AVPacket> _vPktList;
	// ��Ƶ���б����
	CondMutex _vMutex;
	// ��Ƶʱ�ӣ���ǰ��Ƶ����Ӧ��ʱ��ֵ
	double _vTime = 0;
	// ��Ƶ��Դ�Ƿ�����ͷ�
	bool _vCanFree = false;
	// ��Ƶ��Դ�Ƿ�����ͷ�
	bool _vSeekTime = -1;
	// �Ƿ�����Ƶ��
	bool _hasVideo = false;

	// ��ʼ����Ƶ��Ϣ
	int initVideoInfo();
	// ��ʼ����Ƶ���ظ�ʽת��
	int initSws();
	// ������ݰ�����Ƶ���б���
	void addVideoPkt(AVPacket& pkt);
	// �����Ƶ���б�
	void clearVideoPktList();
	// ������Ƶ
	void decodeVideo();

	/************************ ���� ************************/
	// ���װ������
	AVFormatContext* _fmtCtx = nullptr;
	// fmtCtx�Ƿ�����ͷ�
	bool _fmtCtxCanFree = false;
	// ����
	int _volumn = Max;
	// ����
	bool _mute = false;
	// ��ǰ��״̬
	State _state = Stopped;
	// �ļ���
	char _filename[512];
	// �������õĵ�ǰ����ʱ�̣��������seek���ܣ�
	int _seekTime = -1;

	// ��ʼ���������ͽ���������
	int initDecoder(AVCodecContext** decodeCtx,
		AVStream** stream,
		AVMediaType type);
	// �ı�״̬
	void setState(State state);
	// ��ȡ�ļ�����
	void readFile();
	// �ͷ���Դ
	void free();
	void freeAudio();
	void freeVideo();
	// ���ش���
	void fataError();
};

