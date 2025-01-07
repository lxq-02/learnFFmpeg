#include "MainWindow.h"
#include <QTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // 初始化时间
    onTimeChanged(0);
    
    connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::onTimeChanged(unsigned long long ms)
{
    QTime time(0, 0, 0, 0);
    QString text = time.addMSecs(ms).toString("mm::ss.z");
    ui.timeLabel->setText(text.left(7));
}

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread)   // 点击了“开始录音”
    {
        // 开始线程
        _audioThread = new AudioThread(this);
        _audioThread->start();

        connect(_audioThread, &AudioThread::timechanged, this, &MainWindow::onTimeChanged);

        connect(_audioThread, &AudioThread::finished,
            [this]()    // 线程结束
            {
                _audioThread = nullptr;
                ui.audioButton->setText(QStringLiteral("开始录音"));
            });

        // 设置线程文字
        ui.audioButton->setText(QStringLiteral("结束录音"));
    }
    else   // 点击率“结束录音”
    {
        // 结束线程
        //_audioThread->setStop(true);
        _audioThread->requestInterruption();
        _audioThread = nullptr;

        // 设置线程文字
        ui.audioButton->setText(QStringLiteral("开始录音"));
    }
}
