#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>
#include "XVideoView.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}
#pragma comment(lib, "avutil.lib")

#define FILENAME "./video.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV

static XVideoView* view = nullptr;
static AVFrame* frame = nullptr;

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->move(0, 0);
	sdl_width = 1280;
	sdl_height = 720;
	this->resize(sdl_width, sdl_height);

	view = XVideoView::Create();
	view->Init(sdl_width, sdl_height,
		XVideoView::YUV420P, (void*)ui.label->winId());

	// ����frame����ռ�
	frame = av_frame_alloc();
	frame->width = sdl_width;
	frame->height = sdl_height;
	frame->format = AV_PIX_FMT_YUV420P;
	// ����ͼ��ռ� Ĭ��32�ֽڶ���
	auto result = av_frame_get_buffer(frame, 0);
	if (result != 0)
	{
		char buf[1024];
		av_strerror(result, buf, sizeof(buf));
		cout << buf << endl;
	}

	// ���ļ�
	_file.setFileName(FILENAME);
	if (!_file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
	}

	// 5����Ⱦ�߼�
	// ������ʱ��
	_timerId = startTimer(33);
}

SdlQtRGB::~SdlQtRGB()
{
	// 7���ͷ���Դ
	_file.close();
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{
	// YUV420��ʽ��һ������ʹ��1.5���ֽڴ洢��1��Y��0.25��U��0.25��V��
	// yuv420p
	// 4*2
	// yyyy yyyy
	// u    u
	// v    v
	_file.read((char*)frame->data[0], sdl_width * sdl_height);		// Y
	_file.read((char*)frame->data[1], sdl_width * sdl_height / 4);	// U
	_file.read((char*)frame->data[2], sdl_width * sdl_height / 4);	// V

	view->DrawFrame(frame);

	if (view->IsExit())
	{
		view->Close();
		exit(0);
	}

}

void SdlQtRGB::resizeEvent(QResizeEvent* ev)
{
	ui.label->resize(size());
	ui.label->move(0, 0);
	view->Scale(width(), height());
}
