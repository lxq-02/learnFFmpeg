#include "MainWindow.h"
#include <QDebug>	
#include <vector>
#include "YuvPlayer.h"

#define RET(judge, func) \
	if (judge) \
	{ \
		qDebug() << #func << "error" << SDL_GetError(); \
		return; \
	}

#define FILENAME "./dragon_ball.yuv"
#define PIXEL_FORMAT AV_PIX_FMT_YUV420P
#define IMG_W 640
#define IMG_H 480

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

	// ����������
    _player = new YuvPlayer(this);
    int w = 400;
    int h = 400;
    int x = (this->width() - w) >> 1;
    int y = (this->height() - h) >> 1;
    _player->setGeometry(x, y, w, h);

    // ������Ҫ���ŵ�YUV�ļ�
    Yuv yuv =
    {
        FILENAME,
        IMG_W,
        IMG_H,
        PIXEL_FORMAT,
        30
    };
	_player->setYUV(yuv);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_stopButton_clicked()
{
    _player->stop();
}

void MainWindow::on_videoButton_clicked()
{
    // �������ڲ���
    if (_player->isPlaying())
    {
        _player->pause();

        ui->videoButton->setText(QStringLiteral("����"));
    }
    else  // ����û�����ڲ���
    {
        _player->play();
        ui->videoButton->setText(QStringLiteral("��ͣ"));
    }
}