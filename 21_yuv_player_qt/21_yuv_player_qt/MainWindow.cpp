#include "MainWindow.h"
#include <QDebug>	
#include <vector>

static int yuvIdx = 0;
static Yuv yuvs[] =
{
    {
        "./dragon_ball.yuv",
        640, 480,
        AV_PIX_FMT_YUV420P,
        30
    },
    {
        "./in.yuv",
        512, 512,
        AV_PIX_FMT_YUV420P,
        30
    },
    {
        "./dragon_ball_02.yuv",
        888, 480,
        AV_PIX_FMT_YUV420P,
        30
    },
    {
        "./dragon_ball_03.yuv",
        852, 480,
        AV_PIX_FMT_YUV420P,
        30
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

	// ����������
    _player = new YuvPlayer(this);

    // ����������״̬
	connect(_player, &YuvPlayer::stateChanged, this, &MainWindow::on_PlayerStateChanged);

    // ���ò�������λ�úͳߴ�
    int w = 500;
    int h = 500;
    int x = (this->width() - w) / 1;
    int y = (this->height() - h) / 1;
    _player->setGeometry(x, y, w, h);

    // ������Ҫ���ŵ�YUV�ļ�
    _player->setYUV(yuvs[yuvIdx]);
    _player->play();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_stopButton_clicked()
{
    _player->stop();
}

void MainWindow::on_PlayerStateChanged()
{
    if (_player->getState() == YuvPlayer::Playing)
	{
		ui->playButton->setText(QStringLiteral("��ͣ"));
	}
	else
	{
		ui->playButton->setText(QStringLiteral("����"));
	}
}

void MainWindow::on_nextButton_clicked()
{
    int yuvCount = sizeof(yuvs) / sizeof(Yuv);
    yuvIdx = ++yuvIdx % yuvCount;

    _player->stop();
    _player->setYUV(yuvs[yuvIdx]);
    _player->play();
}

void MainWindow::on_playButton_clicked()
{
    // �������ڲ���
    if (_player->isPlaying())
    {
        _player->pause();

    }
    else  // ����û�����ڲ���
    {
        _player->play();
    }
}