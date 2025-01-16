#include "PlayThread.h"
#include <QFile>
#include <QDebug>	
#include <QString>


#define END(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		goto end; \
	}

#define FILENAME "./in.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 512
#define IMG_H 512

PlayThread::PlayThread(QObject *parent)
	: QThread(parent)
{
	// 当监听到线程结束的时候,就调用deleteLater回收内存
	connect(this, &PlayThread::finished,
		this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread()
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
void PlayThread::run()
{
	// 窗口
	SDL_Window* window = nullptr;

	// 渲染上下文
	SDL_Renderer* renderer = nullptr;
	
	// 纹理（直接跟特定驱动程序相关的像素数据）
	SDL_Texture* texture = nullptr;

	// 文件
	QFile file(FILENAME);

	// 初始化子系统
	END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

	// 创建窗口
	window = SDL_CreateWindow(
		// 标题
		QStringLiteral("SDL显示YUV图片").toUtf8().data(),
		// x
		SDL_WINDOWPOS_UNDEFINED,
		// y
		SDL_WINDOWPOS_UNDEFINED,
		// w
		IMG_W,
		// h
		IMG_H,
		SDL_WINDOW_SHOWN
	);
	END(!window, SDL_CreateWindow);

	// 创建渲染上下文
	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		renderer = SDL_CreateRenderer(window, -1, 0);
		END(!renderer, SDL_CreateRenderer);
	}

	// 创建纹理
	texture = SDL_CreateTexture(renderer,
		PIXEL_FORMAT,
		SDL_TEXTUREACCESS_STREAMING,
		IMG_W, IMG_H);
	END(!texture, SDL_CreateTexture);
	
	// 打开文件
	if (!file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
		goto end;
	}

	// 将YUV的像素数据填充到texture
	END(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W), 
		SDL_UpdateTexture);

	// 设置绘制颜色（画笔颜色）
	END(SDL_SetRenderDrawColor(renderer,
		0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

	// 用绘制颜色（画笔颜色）清除渲染目标
	END(SDL_RenderClear(renderer),
		SDL_RenderClear);

	// 拷贝纹理数据到渲染目标（默认是window）
	END(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
		SDL_RenderCopy);

	// 更新所有的渲染操作到屏幕上
	SDL_RenderPresent(renderer);

	// 等待退出时间
	while (!isInterruptionRequested())
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			goto end;
		}
	}

end:
	file.close();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit;
}