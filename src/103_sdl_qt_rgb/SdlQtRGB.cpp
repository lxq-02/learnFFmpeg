#include "SdlQtRGB.h"

static int tmp = 255;

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// 1����ʼ����ϵͳ
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 2����������
	//window = SDL_CreateWindow(QString("sdlʹ��timerEvent��Ⱦ").toUtf8().constData(),
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


	// 3��������Ⱦ��
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 4����������
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,	// �ɼ���
		w, h);
	if (!texture)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 5����Ⱦ�߼�
	// ���ͼ�������
	rgb = new unsigned char[w * h * 4];
	startTimer(10);
}

SdlQtRGB::~SdlQtRGB()
{
	// 6���ӳ��˳�
	SDL_Delay(3000);

	// 7���ͷ���Դ
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

	// 5.1�������ݴ�������
	SDL_UpdateTexture(texture, NULL, rgb, w * 4);

	// 5.2��������Ļ
	SDL_RenderClear(renderer);

	// 5.3������������Ⱦ��
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = w;
	rect.h = h;
	SDL_RenderCopy(renderer, texture,
		NULL, // ԭͼλ�úͳߴ�
		&rect  // Ŀ��λ�úͳߴ�
	);
	// 5.4������Ⱦ���ݸ��µ���Ļ��
	SDL_RenderPresent(renderer);

}
