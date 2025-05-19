#include "SdlQtRGB.h"
#include <QImage>
#include <QDebug>

static int tmp = 255;
static QImage image1;
static QImage image2;

SdlQtRGB::SdlQtRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.label->move(0, 0);
	resize(1300, 600);

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

	image1.load("800_600_red.png");
	image2.load("500_300_blue.png");
	if (image1.isNull() || image2.isNull())
	{
		qDebug() << "not found the pictures" ;
		return;
	}

	w = image1.width() + image2.width();
	h = max(image1.height(), image2.height());
	ui.label->resize(w, h);

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
	// �ϲ�����ͼƬ
	for (int j = 0; j < h; j++)
	{
		int b = j * w * 4;
		if (j < image1.height())
			memcpy(rgb + b, image1.scanLine(j), image1.width() * 4);
		b += image1.width() * 4;
		if (j < image2.height())
			memcpy(rgb + b, image2.scanLine(j), image2.width() * 4);
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
