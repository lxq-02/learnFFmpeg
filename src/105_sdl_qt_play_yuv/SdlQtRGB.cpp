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

	// ��������������
	_widget = new QWidget(this);
	_widget->setGeometry(100, 50, IMG_W, IMG_H);

	// 1����ʼ����ϵͳ
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}

	// 2����������
	window = SDL_CreateWindowFrom((void*)_widget->winId());


	// 3��������Ⱦ��
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return ;
	}


	// 4����������
	texture = SDL_CreateTexture(renderer, PIXEL_FORMAT,
		SDL_TEXTUREACCESS_STREAMING,	// �ɼ���
		IMG_W, IMG_H);
	RET(!texture, SDL_CreateTexture);

	// ���ļ�
	_file.setFileName(FILENAME);
	if (!_file.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << FILENAME;
	}

	// 5����Ⱦ�߼�
	// ������ʱ��
	_timerId = startTimer(33);
}

SdlQtRGB::~SdlQtRGB()
{

	// 7���ͷ���Դ
	_file.close();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{
	// YUV420��ʽ��һ������ʹ��1.5���ֽڴ洢��1��Y��0.25��U��0.25��V��
	int imgSize = IMG_W * IMG_H * 3 / 2;
	std::vector<char> data(imgSize);

	if (_file.read(data.data(), imgSize) > 0)
	{
		// ��YUV������������䵽texture
		RET(SDL_UpdateTexture(texture, nullptr, data.data(), IMG_W), SDL_UpdateTexture);


		// ���û�����ɫ��������ɫ��
		RET(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

		// �û�����ɫ��������ɫ�������ȾĿ��
		RET(SDL_RenderClear(renderer), SDL_RenderClear);

		// �����������ݵ���ȾĿ�꣨Ĭ����window��
		RET(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

		// �������е���Ⱦ��������Ļ��
		SDL_RenderPresent(renderer);
	}
	else
	{
		// �ļ��Ѿ���ȡ���
		killTimer(_timerId);
	}

}