#include "PlayThread.h"
#include <QFile>
#include <QDebug>	
#include <QString>

#include <SDL.h>

#define END(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		goto end; \
	}


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
	// ��������
	SDL_Surface* surface = nullptr;

	// ����
	SDL_Window* window = nullptr;

	// ��Ⱦ������
	SDL_Renderer* renderer = nullptr;
	
	// ����ֱ�Ӹ��ض�����������ص��������ݣ�
	SDL_Texture* texture = nullptr;

	// ���ο�
	SDL_Rect srcRect = { 0, 0, 512, 512 };
	SDL_Rect dstRect = { 200, 200, 100, 100 };
	SDL_Rect rect;


	// ��ʼ����ϵͳ
	END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

	// ����BMP
	surface = SDL_LoadBMP("./in.bmp");
	END(!surface, SDL_LoadBMP);

	// ��������
	window = SDL_CreateWindow(
		// ����
		QStringLiteral("SDL��ʾBMPͼƬ").toUtf8().data(),
		// x
		SDL_WINDOWPOS_UNDEFINED,
		// y
		SDL_WINDOWPOS_UNDEFINED,
		// w
		surface->w,
		// h
		surface->h,
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
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	END(!texture, SDL_CreateTextureFromSurface);
	
	// ��һ����ɫ�ľ��ο�
	// ���û�����ɫ
	END(SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColoe);
	// ���ƾ���
	rect = { 0, 0, 50, 50 };
	// ����
	END(SDL_RenderFillRect(renderer, &rect), SDL_RenderFillRect);

	// ���û�����ɫ��������ɫ��
	END(SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
	// �û�����ɫ��������ɫ�������ȾĿ�� ��ǰ�Ļ�ɫ
	END(SDL_RenderClear(renderer), SDL_RenderClear);

	// �����������ݵ���ȾĿ�꣨Ĭ����window��
	END(SDL_RenderCopy(renderer, texture, &srcRect, &dstRect), SDL_RenderCopy);

	// �������е���Ⱦ��������Ļ��
	SDL_RenderPresent(renderer);

	SDL_Delay(2000);

end:
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit;
}
