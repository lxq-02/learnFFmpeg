#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>
#include "XVideoView.h"

#define RET(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		return; \
	}

#define FILENAME "./video.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV

static XVideoView* view = nullptr;

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
	int imgSize = sdl_width * sdl_height * 3 / 2;
	std::vector<unsigned char> data(imgSize);
	if (_file.read((char*)data.data(), imgSize) > 0)
	{
		view->Draw(data.data(), sdl_width);
	}
	else
	{
		// �ļ��Ѿ���ȡ���
		killTimer(_timerId);
	}

}

void SdlQtRGB::resizeEvent(QResizeEvent* ev)
{
	ui.label->resize(size());
	ui.label->move(0, 0);
	view->Scale(width(), height());
}
