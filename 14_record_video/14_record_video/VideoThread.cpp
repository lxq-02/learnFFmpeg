#include "VideoThread.h"
#include <QFile>
#include <QDebug>	

extern "C"
{
	// 设备
#include <libavdevice/avdevice.h>
// 格式
#include <libavformat/avformat.h>
// 工具（比如错误处理）
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
	// 当监听到线程结束的时候,就调用deleteLater回收内存
	connect(this, &VideoThread::finished,
		this, &VideoThread::deleteLater);
}

VideoThread::~VideoThread()
{
	// 断开所有的连接
	disconnect();
	// 回收内存之前，正常结束线程
	requestInterruption();
	// 安全退出
	quit();
	wait();
	qDebug() << this << QStringLiteral("析构（内存被回收)");
}

// 当线程启动的时候（start），会调用run
// run函数中的代码是在子线程中执行的
// 耗时操作应该放在run函数中
void VideoThread::run()
{
	qDebug() << this << QStringLiteral("开始执行------------------");
	// 2、获取输入格式对象
	AVInputFormat* fmt = av_find_input_format(FMT_NAME);
	if (!fmt)
	{
		qDebug() << "av_find_input_format error" << FMT_NAME;
		return;
	}

	// 3、打开输入设备
	// 格式上下文
	AVFormatContext* ctx = nullptr;
	// 传递给输入设备的参数
	AVDictionary* options = nullptr;
	av_dict_set(&options, "video_size", "640x480", 0);
	av_dict_set(&options, "pixel_format", "yuyv422", 0);
	av_dict_set(&options, "framerate", "30", 0);

	// 打开输入设备
	int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
	if (ret < 0)
	{
		ERROR_BUF(ret);
		qDebug() << "avformat_open_input error" << errbuf;
		return;
	}

	// 4、打开输出文件
	QFile file(FILENAME);
	if (!file.open(QFile::WriteOnly))
	{
		qDebug() << "file open error" << FILENAME;

		// 关闭输入设备
		avformat_close_input(&ctx);
		return;
	}

	// 5、采集视频数据
	// 计算每一帧的大小
	AVCodecParameters* params = ctx->streams[0]->codecpar;
	int imageSize = av_image_get_buffer_size((AVPixelFormat)params->format, 
												params->width,
												params->height, 
												1);

	// 数据包
	AVPacket* pkt = av_packet_alloc();
	while (!isInterruptionRequested())
	{
		// 不断采集数据
		ret = av_read_frame(ctx, pkt);

		if (ret == 0) // 读取成功
		{
			// 将数据写入文件
			file.write((const char*)pkt->data, imageSize);
			/*
			这里要使用imageSize, 而不是pkt->size。
			pkt->size有可能比imageSize大（比如在Mac平台),
			使用pkt->size会导致写入一些多余数据到YUV文件中，
			进而导致YUV内容无法正常播放
			*/

			// 释放资源
			av_packet_unref(pkt);
		}
		else if (ret == AVERROR(EAGAIN)) // 资源临时不可用
		{
			continue;
		}
		else // 其他错误
		{
			ERROR_BUF(ret);
			qDebug() << "av_read_frame error" << errbuf;
			break;
		}
	}

	// 释放资源
	av_packet_free(&pkt);

	// 关闭文件
	file.close();

	// 关闭设备
	avformat_close_input(&ctx);

	qDebug() << this << QStringLiteral("正常结束-----------");
}
