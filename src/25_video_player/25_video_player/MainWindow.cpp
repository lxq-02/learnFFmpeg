#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <VideoWidget.h>

// 构造、析构
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass) 
{
    ui->setupUi(this);
    
    // 注册信号的参数类型，保证能够发出信号
    qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec&");

    // 创建播放器
    _player = new VideoPlayer();
    connect(_player, &VideoPlayer::stateChanged,
        this, &MainWindow::onPlayerStateChanged);
    connect(_player, &VideoPlayer::timeChanged,
        this, &MainWindow::onPlayerTimeChanged);
    connect(_player, &VideoPlayer::initFinished,
        this, &MainWindow::onPlayerInitFinished);
    connect(_player, &VideoPlayer::playFailed,
        this, &MainWindow::onPlayerPlayFailed);

    connect(_player, &VideoPlayer::frameDecoded,
        ui->videoWidget, &VideoWidget::onPlayerFrameDecoded);
    connect(_player, &VideoPlayer::stateChanged,
        ui->videoWidget, &VideoWidget::onPlayerStateChanged);

    // 监听事件滑块的点击
    connect(ui->timeSlider, &VideoSlider::clicked,
        this, &MainWindow::onSliderClicked);

    // 设置音量滑块的范围
    ui->volumnSlider->setRange(VideoPlayer::Volumn::Min, VideoPlayer::Volumn::Max);
    ui->volumnSlider->setValue(ui->volumnSlider->maximum() >> 3);
}

MainWindow::~MainWindow() 
{
    delete ui;
    delete _player;
}

void MainWindow::onPlayerTimeChanged(VideoPlayer* player)
{
    ui->timeSlider->setValue(player->getTime());
}

void MainWindow::onPlayerInitFinished(VideoPlayer* player)
{
    int duration = player->getDuration();

    // 设置slider的范围
    ui->timeSlider->setRange(0, duration);

    // 设置label的文字
    ui->durationLabel->setText(getTimeText(duration));
}

void MainWindow::onPlayerPlayFailed(VideoPlayer* player)
{
    QMessageBox::critical(nullptr, "提示", "播放失败");
}

void MainWindow::onSliderClicked(VideoSlider* slider)
{
    _player->setTime(slider->value());
}

void MainWindow::onPlayerStateChanged(VideoPlayer* player)
{
    VideoPlayer::State state = player->getState();
    if (state == VideoPlayer::Playing)
    {
        ui->playBtn->setText("暂停");
    }
    else
    {
        ui->playBtn->setText("播放");
    }

    if (state == VideoPlayer::Stopped)
    {
        ui->playBtn->setEnabled(false);
        ui->stopBtn->setEnabled(false);
        ui->timeSlider->setEnabled(false);
        ui->volumnSlider->setEnabled(false);
        ui->muteBtn->setEnabled(false);

        ui->durationLabel->setText(getTimeText(0));
        ui->timeSlider->setValue(0);

        // 显示打开文件的页面
        ui->playWidget->setCurrentWidget(ui->openFilePage);
    }
    else
    {
        ui->playBtn->setEnabled(true);
        ui->stopBtn->setEnabled(true);
        ui->timeSlider->setEnabled(true);
        ui->volumnSlider->setEnabled(true);
        ui->muteBtn->setEnabled(true);

        // 显示播放视频的页面
        ui->playWidget->setCurrentWidget(ui->videoPage);
    }
}

void MainWindow::on_stopBtn_clicked()
{
    _player->stop();
}

void MainWindow::on_openFileBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("选择多媒体文件"),
        "./",
        QStringLiteral("多媒体文件(*.mp4 *.avi *.mkv *.mp3 *.aac)"));
    if (filename.isEmpty()) return;

    // 开始播放打开的文件
    _player->setFilename(filename);
    _player->play();
}

void MainWindow::on_timeSlider_valueChanged(int value)
{
    ui->timeLabel->setText(getTimeText(value));
}

void MainWindow::on_volumnSlider_valueChanged(int value)
{
    ui->volumnLabel->setText(QString("%1").arg(value));
    _player->setVolumn(value);
}

void MainWindow::on_playBtn_clicked()
{
    VideoPlayer::State state = _player->getState();
    if (state == VideoPlayer::Playing)
    {
        _player->pause();
    }
    else
    {
        _player->play();
    }
}

void MainWindow::on_muteBtn_clicked()
{
    if (_player->isMute())
    {
        _player->setMute(false);
        ui->muteBtn->setText("静音");
    }
    else
    {
        _player->setMute(true);
        ui->muteBtn->setText("开音");
    }
}

QString MainWindow::getTimeText(int value)
{
    QLatin1Char fill = QLatin1Char('0');
    return QString("%1:%2:%3")
        .arg(value / 3600, 2, 10, fill)
        .arg((value / 60) % 60, 2, 10, fill)
        .arg(value % 60, 2, 10, fill);
}
