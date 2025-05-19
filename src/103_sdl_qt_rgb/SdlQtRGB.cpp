#include "SdlQtRGB.h"

static int tmp = 255;

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// 1、初始化子系统
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 2、创建窗口
	//window = SDL_CreateWindow(QString("sdl使用timerEvent渲染").toUtf8().constData(),
	//	0,
	//	0,
	//	w,
	//	h,
	//	SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//if (window == NULL)
	//{
	//	std::cout << SDL_GetError() << std::endl;
	//	return ;
	//}
	window = SDL_CreateWindowFrom((void*)ui.label->winId());


	// 3、创建渲染器
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 4、创建纹理
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,	// 可加锁
		w, h);
	if (!texture)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 5、渲染逻辑
	// 存放图像的数据
	rgb = new unsigned char[w * h * 4];
	startTimer(10);
}

SdlQtRGB::~SdlQtRGB()
{
	// 6、延迟退出
	SDL_Delay(3000);

	// 7、释放资源
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{
	tmp--;
	for (int j = 0; j < h; j++)
	{
		int b = j * w * 4;
		for (int i = 0; i < w * 4; i += 4)
		{
			// ARGB
			rgb[b + i] = 0;		// B
			rgb[b + i + 1] = 0;	// G
			rgb[b + i + 2] = tmp;	// R
			rgb[b + i + 3] = 0;	// A
		}
	}

	// 5.1、将数据传入纹理
	SDL_UpdateTexture(texture, NULL, rgb, w * 4);

	// 5.2、清理屏幕
	SDL_RenderClear(renderer);

	// 5.3、复制纹理到渲染器
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = w;
	rect.h = h;
	SDL_RenderCopy(renderer, texture,
		NULL, // 原图位置和尺寸
		&rect  // 目标位置和尺寸
	);
	// 5.4、将渲染内容更新到屏幕上
	SDL_RenderPresent(renderer);

}
