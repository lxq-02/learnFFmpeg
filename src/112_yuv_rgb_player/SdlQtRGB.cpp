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

using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
}
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "winmm.lib")

#define FILENAME "./video.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV

static XVideoView* view = nullptr;
static AVFrame* frame = nullptr;
static long long yuv_size = 0;
static QLabel* view_fps = nullptr;	// 显示fps控件
static QSpinBox* set_fps = nullptr; // 设置fps控件
int fps = 0;

void MSleep(unsigned int ms)
{
	auto beg = chrono::high_resolution_clock::now();
	while (true)
	{
		auto now = chrono::high_resolution_clock::now();
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - beg).count();
		if (elapsed >= ms) break;

		// 剩余时间 >1ms 才 sleep
		if (ms - elapsed > 1) 
		{
			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
}

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// 提高计时器精度到 1ms
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(1);

	connect(this, &SdlQtRGB::ViewS, this, &SdlQtRGB::View);

	_th = std::thread(&SdlQtRGB::Main, this);
}

SdlQtRGB::~SdlQtRGB()
{
	// 7、释放资源
	_file.close();

	_is_exit = true;
	// 等待渲染线程退出
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

void SdlQtRGB::View()	// 显示的槽函数
{
}

void SdlQtRGB::Main()
{
	while (!_is_exit)
	{
		emit ViewS();
		MSleep(1);
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
}
