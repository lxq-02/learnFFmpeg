#include "VideoThread.h"
#include <QFile>
#include <QDebug>	

extern "C"
{
	// �豸
#include <libavdevice/avdevice.h>
// ��ʽ
#include <libavformat/avformat.h>
// ���ߣ����������
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
	#define FMT_NAME "dshow"
	#define DEVICE_NAME "video=Integrated Camera"
	#define FILENAME "./out.yuv"
#else
	#define FMT_NAME "avfoundation"
	#define DEVICE_NAME "0"
	#define FILENAME "/Users/mj/Desktop/out.yuv"
#endif

#define ERROR_BUF(ret) \
	char errbuf[1024]; \
	av_strerror(ret, errbuf, sizeof(errbuf));


VideoThread::VideoThread(QObject *parent)
	: QThread(parent)
{
	// ���������߳̽�����ʱ��,�͵���deleteLater�����ڴ�
	connect(this, &VideoThread::finished,
		this, &VideoThread::deleteLater);
}

VideoThread::~VideoThread()
{
	// �Ͽ����е�����
	disconnect();
	// �����ڴ�֮ǰ�����������߳�
	requestInterruption();
	// ��ȫ�˳�
	quit();
	wait();
	qDebug() << this << QStringLiteral("�������ڴ汻����)");
}

// ���߳�������ʱ��start���������run
// run�����еĴ����������߳���ִ�е�
// ��ʱ����Ӧ�÷���run������
void VideoThread::run()
{
	qDebug() << this << QStringLiteral("��ʼִ��------------------");
	// 2����ȡ�����ʽ����
	AVInputFormat* fmt = av_find_input_format(FMT_NAME);
	if (!fmt)
	{
		qDebug() << "av_find_input_format error" << FMT_NAME;
		return;
	}

	// 3���������豸
	// ��ʽ������
	AVFormatContext* ctx = nullptr;
	// ���ݸ������豸�Ĳ���
	AVDictionary* options = nullptr;
	av_dict_set(&options, "video_size", "640x480", 0);
	av_dict_set(&options, "pixel_format", "yuyv422", 0);
	av_dict_set(&options, "framerate", "30", 0);

	// �������豸
	int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
	if (ret < 0)
	{
		ERROR_BUF(ret);
		qDebug() << "avformat_open_input error" << errbuf;
		return;
	}

	// 4��������ļ�
	QFile file(FILENAME);
	if (!file.open(QFile::WriteOnly))
	{
		qDebug() << "file open error" << FILENAME;

		// �ر������豸
		avformat_close_input(&ctx);
		return;
	}

	// 5���ɼ���Ƶ����
	// ����ÿһ֡�Ĵ�С
	AVCodecParameters* params = ctx->streams[0]->codecpar;
	int imageSize = av_image_get_buffer_size((AVPixelFormat)params->format, 
												params->width,
												params->height, 
												1);

	// ���ݰ�
	AVPacket* pkt = av_packet_alloc();
	while (!isInterruptionRequested())
	{
		// ���ϲɼ�����
		ret = av_read_frame(ctx, pkt);

		if (ret == 0) // ��ȡ�ɹ�
		{
			// ������д���ļ�
			file.write((const char*)pkt->data, imageSize);
			/*
			����Ҫʹ��imageSize, ������pkt->size��
			pkt->size�п��ܱ�imageSize�󣨱�����Macƽ̨),
			ʹ��pkt->size�ᵼ��д��һЩ�������ݵ�YUV�ļ��У�
			��������YUV�����޷���������
			*/

			// �ͷ���Դ
			av_packet_unref(pkt);
		}
		else if (ret == AVERROR(EAGAIN)) // ��Դ��ʱ������
		{
			continue;
		}
		else // ��������
		{
			ERROR_BUF(ret);
			qDebug() << "av_read_frame error" << errbuf;
			break;
		}
	}

	// �ͷ���Դ
	av_packet_free(&pkt);

	// �ر��ļ�
	file.close();

	// �ر��豸
	avformat_close_input(&ctx);

	qDebug() << this << QStringLiteral("��������-----------");
}
