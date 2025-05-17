#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

    connect(ui->videoButton, &QPushButton::clicked, this, &MainWindow::on_videoButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_videoButton_clicked()
{
    if (!videoThread)   // 点击了开始录视频
    {
        // 开启线程
		videoThread = new VideoThread(this);
		videoThread->start();

        connect(videoThread, &VideoThread::finished, [this]()
            {
                // 线程结束
                videoThread = nullptr;
                ui->videoButton->setText(QStringLiteral("开始录视频"));
            });

        // 设置按钮文字
        ui->videoButton->setText(QStringLiteral("结束录视频"));
    }
    else // 点击了“结束录视频"
    {
        // 结束线程
        videoThread->requestInterruption();
        videoThread = nullptr;

        // 设置按钮文字
        ui->videoButton->setText(QStringLiteral("开始录视频"));
    }
}
