#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>
#include <thread>
#include "XVideoView.h"
#include <sstream>
#include <windows.h>
#include <mmsystem.h>
#include <QSpinBox>
#include <QFileDialog>
#include <iostream>
#include <vector>
#include <QString>
#include <sstream>

using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
}
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "winmm.lib")

#define FILENAME "./video.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV

static std::vector<XVideoView*> views;

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// ��߼�ʱ�����ȵ� 1ms
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(1);

	connect(this, &SdlQtRGB::ViewS, this, &SdlQtRGB::View);
	connect(ui.open1, &QPushButton::clicked, this, &SdlQtRGB::Open1);
	connect(ui.open2, &QPushButton::clicked, this, &SdlQtRGB::Open2);
	views.push_back(XVideoView::Create());
	views.push_back(XVideoView::Create());
	views[0]->set_win_id((void*)ui.video1->winId());
	views[1]->set_win_id((void*)ui.video2->winId());

	
	_th = std::thread(&SdlQtRGB::Main, this);
}

SdlQtRGB::~SdlQtRGB()
{
	// 7���ͷ���Դ
	_file.close();

	_is_exit = true;
	// �ȴ���Ⱦ�߳��˳�
	_th.join();

	timeEndPeriod(1);
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{


}

void SdlQtRGB::resizeEvent(QResizeEvent* ev)
{
	//ui.label->resize(size());
	//ui.label->move(0, 0);
	//view->Scale(width(), height());
}

void SdlQtRGB::View()	// ��ʾ�Ĳۺ���
{
	// ����ϴ���Ⱦ��ʱ���
	static int last_pts[32] = { 0 };
	static int fps_arr[32] = { 0 };
	fps_arr[0] = ui.fps1->value();
	fps_arr[1] = ui.fps2->value();
	for (int i = 0; i < views.size(); ++i)
	{
		if (fps_arr[i] <= 0) continue;
		// ��Ҫ���ʱ��
		int ms = 1000 / fps_arr[i];

		// �ж��Ƿ��˿���Ⱦʱ��
		if (NowMs() - last_pts[i] < ms)
			continue;
		last_pts[i] = NowMs();

		auto frame = views[i]->Read();
		if (!frame) continue;
		views[i]->DrawFrame(frame);
		// ��ʾfps
		stringstream ss;
		ss << "fps: " << views[i]->render_fps();
		if (i == 0)
			ui.show_fps1->setText(ss.str().c_str());
		else
			ui.show_fps1_2->setText(ss.str().c_str());
	}
}

void SdlQtRGB::Main()
{
	while (!_is_exit)
	{
		emit ViewS();
		MSleep(10);
	}
}

void SdlQtRGB::Open1()
{
	Open(0);
}

void SdlQtRGB::Open2()
{
	Open(1);
}

void SdlQtRGB::Open(int i)
{
	QFileDialog fd;
	auto filename = fd.getOpenFileName();
	if (filename.isEmpty()) return;
	cout << filename.toLocal8Bit().data() << endl;

	// ���ļ�
	if (!views[i]->Open(filename.toStdString()))
	{
		return;
	}
	// ��ʼ�����ں�����
	int w = 0;
	int h = 0;
	QString pix = 0; // YUV420P RGBA
	if (i == 0)
	{
		w = ui.width1->value();
		h = ui.height1->value();
		pix = ui.pix1->currentText();
	}
	else
	{
		w = ui.width2->value();
		h = ui.height2->value();
		pix = ui.pix2->currentText();
	}
	XVideoView::Format fmt = XVideoView::YUV420P;
	if (pix == "YUV420P")
	{
		fmt = XVideoView::YUV420P;
	}
	else if (pix == "RGBA")
	{
		fmt = XVideoView::RGBA;
	}
	else if (pix == "ARGB")
	{
		fmt = XVideoView::ARGB;
	}
	else if (pix == "BGRA")
	{
		fmt = XVideoView::BGRA;
	}
	else if (pix == "RGB24")
	{
		fmt = XVideoView::RGB24;
	}
	views[i]->Init(w, h, fmt);
}
