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
	// ���������߳̽�����ʱ��,�͵���deleteLater�����ڴ�
	connect(this, &PlayThread::finished,
		this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread()
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
void PlayThread::run()
{
	// ����
	SDL_Window* window = nullptr;

	// ��Ⱦ������
	SDL_Renderer* renderer = nullptr;
	
	// ����ֱ�Ӹ��ض�����������ص��������ݣ�
	SDL_Texture* texture = nullptr;

	// �ļ�
	QFile file(FILENAME);

	// ��ʼ����ϵͳ
	END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

	// ��������
	window = SDL_CreateWindow(
		// ����
		QStringLiteral("SDL��ʾYUVͼƬ").toUtf8().data(),
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

	// ������Ⱦ������
	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		renderer = SDL_CreateRenderer(window, -1, 0);
		END(!renderer, SDL_CreateRenderer);
	}

	// ��������
	texture = SDL_CreateTexture(renderer,
		PIXEL_FORMAT,
		SDL_TEXTUREACCESS_STREAMING,
		IMG_W, IMG_H);
	END(!texture, SDL_CreateTexture);
	
	// ���ļ�
	if (!file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
		goto end;
	}

	// ��YUV������������䵽texture
	END(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W), 
		SDL_UpdateTexture);

	// ���û�����ɫ��������ɫ��
	END(SDL_SetRenderDrawColor(renderer,
		0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

	// �û�����ɫ��������ɫ�������ȾĿ��
	END(SDL_RenderClear(renderer),
		SDL_RenderClear);

	// �����������ݵ���ȾĿ�꣨Ĭ����window��
	END(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
		SDL_RenderCopy);

	// �������е���Ⱦ��������Ļ��
	SDL_RenderPresent(renderer);

	// �ȴ��˳�ʱ��
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