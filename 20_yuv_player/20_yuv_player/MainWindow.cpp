#include "MainWindow.h"
#include <QDebug>	
#include <vector>

#define RET(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		return; \
	}

#define FILENAME "./dragon_ball.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 640
#define IMG_H 480

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

    // ��������������
    _widget = new QWidget(this);
    _widget->setGeometry(100, 50, IMG_W, IMG_H);

    // ��ʼ����ϵͳ
    RET(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    // ��������
    _window = SDL_CreateWindowFrom((void*)_widget->winId());
    RET(!_window, SDL_CreateWindowFrom);

	// ������Ⱦ������
	_renderer = SDL_CreateRenderer(_window, -1, 
        SDL_RENDERER_SOFTWARE |
        SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer)
    {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
        RET(!_renderer, SDL_CreateRenderer);
    }

    // ��������
    _texture = SDL_CreateTexture(_renderer,
        PIXEL_FORMAT,
        SDL_TEXTUREACCESS_STREAMING,
        IMG_W, IMG_H);
    RET(!_texture, SDL_CreateTexture);

    // ���ļ�
    _file.setFileName(FILENAME);
    if (!_file.open(QFile::ReadOnly))
    {
        qDebug() << "file open error" << FILENAME;
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    _file.close();
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit;
}

void MainWindow::on_videoButton_clicked()
{
    // ������ʱ��
    _timerId = startTimer(33);
}

// ÿ��һ��ʱ��ͻ����
void MainWindow::timerEvent(QTimerEvent* event)
{
    int imgSize = IMG_W * IMG_H * 3 / 2;
    std::vector<char> data(imgSize);

    if (_file.read(data.data(), imgSize) > 0)
    {
        // ��YUV������������䵽texture
        RET(SDL_UpdateTexture(_texture, nullptr, data.data(), IMG_W), SDL_UpdateTexture);

        // ���û�����ɫ��������ɫ��
        RET(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

        // �û�����ɫ��������ɫ�������ȾĿ��
        RET(SDL_RenderClear(_renderer), SDL_RenderClear);

        // �����������ݵ���ȾĿ�꣨Ĭ����window��
        RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

        // �������е���Ⱦ��������Ļ��
        SDL_RenderPresent(_renderer);
    }
    else
    {
        // �ļ��Ѿ���ȡ���
        killTimer(_timerId);
    }
}
