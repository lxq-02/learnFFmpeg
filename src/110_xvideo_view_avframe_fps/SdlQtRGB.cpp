#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>
#include <thread>
#include "XVideoView.h"
#include <sstream>
#include <windows.h>
#include <mmsystem.h>
#include <QSpinBox>

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
	this->move(0, 0);
	sdl_width = 1280;
	sdl_height = 720;
	this->resize(sdl_width, sdl_height);

	// 提高计时器精度到 1ms
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(1);

	connect(this, &SdlQtRGB::ViewS, this, &SdlQtRGB::View);

	// 显示fps的控件
	view_fps = new QLabel(this);
	view_fps->setText("fps:100");

	// 设置fps
	set_fps = new QSpinBox(this);
	set_fps->move(200, 0);
	set_fps->setValue(25);
	set_fps->setRange(1, 300);

	view = XVideoView::Create();
	view->Init(sdl_width, sdl_height,
		XVideoView::YUV420P, (void*)ui.label->winId());

	// 生成frame对象空间
	frame = av_frame_alloc();
	frame->width = sdl_width;
	frame->height = sdl_height;
	frame->format = AV_PIX_FMT_YUV420P;
	// 生成图像空间 默认32字节对齐
	auto result = av_frame_get_buffer(frame, 0);
	if (result != 0)
	{
		char buf[1024];
		av_strerror(result, buf, sizeof(buf));
		cout << buf << endl;
	}

	// 打开文件
	_file.open(FILENAME, ios::binary);
	if (!_file)
	{
		qDebug() << "file open error" << FILENAME;
	}
	_file.seekg(0, ios::end);	// 移到文件结尾
	yuv_size = _file.tellg();	// 获取文件大小
	_file.seekg(0, ios::beg);	// 移到文件开头

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
	// YUV420格式：一个像素使用1.5个字节存储（1个Y，0.25个U，0.25个V）
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

void SdlQtRGB::View()	// 显示的槽函数
{
	_file.read((char*)frame->data[0], sdl_width * sdl_height);		// Y
	_file.read((char*)frame->data[1], sdl_width * sdl_height / 4);	// U
	_file.read((char*)frame->data[2], sdl_width * sdl_height / 4);	// V
	if (_file.tellg() == yuv_size)
	{
		_file.seekg(0, ios::beg);
	}
	if (view->IsExit())
	{
		view->Close();
		exit(0);
	}

	view->DrawFrame(frame);

	stringstream ss;
	ss << "fps: " << view->render_fps();

	// 只能在槽函数中使用
	view_fps->setText(ss.str().c_str());
	fps = set_fps->value();
}

void SdlQtRGB::Main()
{
	while (!_is_exit)
	{
		emit ViewS();
		if (fps > 0)
			MSleep(1000 / fps);
		else
			MSleep(10);
	}
}
