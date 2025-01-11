#include "MainWindow.h"
#include <QTime>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
    //connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread)   // 点击了“开始录音”
    {
        // 开始线程
        _audioThread = new AudioThread(this);
        _audioThread->start();

        connect(_audioThread, &AudioThread::finished,
            [this]()    // 线程结束
            {
                _audioThread = nullptr;
                ui.audioButton->setText(QStringLiteral("开始录视频"));
            });

        // 设置线程文字
        ui.audioButton->setText(QStringLiteral("结束录视频"));
    }
    else   // 点击率“结束录音”
    {
        // 结束线程
        _audioThread->requestInterruption();
        _audioThread->wait(); // 等待线程完全退出
        delete _audioThread;  // 手动销毁线程对象
        _audioThread = nullptr;

        // 设置线程文字
        ui.audioButton->setText(QStringLiteral("开始视频 "));
    }
}
