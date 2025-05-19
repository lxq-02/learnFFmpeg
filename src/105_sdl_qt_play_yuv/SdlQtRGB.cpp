#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>

#define RET(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		return; \
	}

#define FILENAME "./video.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 1280
#define IMG_H 720

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.label->move(0, 0);
	resize(1300, 600);

	// 创建播放器界面
	_widget = new QWidget(this);
	_widget->setGeometry(100, 50, IMG_W, IMG_H);

	// 1、初始化子系统
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 2、创建窗口
	window = SDL_CreateWindowFrom((void*)_widget->winId());


	// 3、创建渲染器
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}


	// 4、创建纹理
	texture = SDL_CreateTexture(renderer, PIXEL_FORMAT,
		SDL_TEXTUREACCESS_STREAMING,	// 可加锁
		IMG_W, IMG_H);
	RET(!texture, SDL_CreateTexture);

	// 打开文件
	_file.setFileName(FILENAME);
	if (!_file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
	}

	// 5、渲染逻辑
	// 开启定时器
	_timerId = startTimer(33);
}

SdlQtRGB::~SdlQtRGB()
{

	// 7、释放资源
	_file.close();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{
	// YUV420格式：一个像素使用1.5个字节存储（1个Y，0.25个U，0.25个V）
	int imgSize = IMG_W * IMG_H * 3 / 2;
	std::vector<char> data(imgSize);

	if (_file.read(data.data(), imgSize) > 0)
	{
		// 将YUV的像素数据填充到texture
		RET(SDL_UpdateTexture(texture, nullptr, data.data(), IMG_W), SDL_UpdateTexture);


		// 设置绘制颜色（画笔颜色）
		RET(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

		// 用绘制颜色（画笔颜色）清除渲染目标
		RET(SDL_RenderClear(renderer), SDL_RenderClear);

		// 拷贝纹理数据到渲染目标（默认是window）
		RET(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

		// 更新所有的渲染操作到屏幕上
		SDL_RenderPresent(renderer);
	}
	else
	{
		// 文件已经读取完毕
		killTimer(_timerId);
	}

}