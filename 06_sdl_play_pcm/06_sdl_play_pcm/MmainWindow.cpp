#include "MmainWindow.h"
#include <SDL.h>
#include <qDebug>

MmainWindow::MmainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MmainWindowClass())
{
    ui->setupUi(this);

    connect(ui->playButton, &QPushButton::clicked, this, &MmainWindow::on_playButton_clicked);
}

MmainWindow::~MmainWindow()
{
    delete ui;
}

void MmainWindow::showVersion()
{
    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}

void MmainWindow::on_playButton_clicked()
{
    if (_playThread) // 停止播放
    {
        _playThread->requestInterruption();
        _playThread = nullptr;
        ui->playButton->setText(QStringLiteral("开始播放"));
    }
    else // 开始播放
    {
        _playThread = new playThread(this);
        _playThread->start();
        // 监听线程的结束
        connect(_playThread, &playThread::finished,
            [this]()
            {
                _playThread = nullptr;
                ui->playButton->setText(QStringLiteral("开始播放"));
            });
        ui->playButton->setText(QStringLiteral("停止播放"));
    }
}
