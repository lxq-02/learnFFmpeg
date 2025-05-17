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

    // 创建播放器界面
    _widget = new QWidget(this);
    _widget->setGeometry(100, 50, IMG_W, IMG_H);

    // 初始化子系统
    RET(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    // 创建窗口
    _window = SDL_CreateWindowFrom((void*)_widget->winId());
    RET(!_window, SDL_CreateWindowFrom);

	// 创建渲染上下文
	_renderer = SDL_CreateRenderer(_window, -1, 
        SDL_RENDERER_SOFTWARE |
        SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer)
    {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
        RET(!_renderer, SDL_CreateRenderer);
    }

    // 创建纹理
    _texture = SDL_CreateTexture(_renderer,
        PIXEL_FORMAT,
        SDL_TEXTUREACCESS_STREAMING,
        IMG_W, IMG_H);
    RET(!_texture, SDL_CreateTexture);

    // 打开文件
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
    // 开启定时器
    _timerId = startTimer(33);
}

// 每隔一段时间就会调用
void MainWindow::timerEvent(QTimerEvent* event)
{
    int imgSize = IMG_W * IMG_H * 3 / 2;
    std::vector<char> data(imgSize);

    if (_file.read(data.data(), imgSize) > 0)
    {
        // 将YUV的像素数据填充到texture
        RET(SDL_UpdateTexture(_texture, nullptr, data.data(), IMG_W), SDL_UpdateTexture);

        // 设置绘制颜色（画笔颜色）
        RET(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

        // 用绘制颜色（画笔颜色）清除渲染目标
        RET(SDL_RenderClear(_renderer), SDL_RenderClear);

        // 拷贝纹理数据到渲染目标（默认是window）
        RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

        // 更新所有的渲染操作到屏幕上
        SDL_RenderPresent(_renderer);
    }
    else
    {
        // 文件已经读取完毕
        killTimer(_timerId);
    }
}
