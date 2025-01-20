#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <VideoWidget.h>

// ���졢����
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass) 
{
    ui->setupUi(this);
    
    // ע���źŵĲ������ͣ���֤�ܹ������ź�
    qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec&");

    // ����������
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

    // �����¼�����ĵ��
    connect(ui->timeSlider, &VideoSlider::clicked,
        this, &MainWindow::onSliderClicked);

    // ������������ķ�Χ
    ui->volumnSlider->setRange(VideoPlayer::Volumn::Min, VideoPlayer::Volumn::Max);
    ui->volumnSlider->setValue(ui->volumnSlider->maximum() >> 3);
}

MainWindow::~MainWindow() 
{
    delete ui;
    delete _player;
}

#pragma mark - ˽�з���
void MainWindow::onPlayerTimeChanged(VideoPlayer* player)
{
    ui->timeSlider->setValue(player->getTime());
}

void MainWindow::onPlayerInitFinished(VideoPlayer* player)
{
    int duration = player->getDuration();

    // ����slider�ķ�Χ
    ui->timeSlider->setRange(0, duration);

    // ����label������
    ui->durationLabel->setText(getTimeText(duration));
}

void MainWindow::onPlayerPlayFailed(VideoPlayer* player)
{
    QMessageBox::critical(nullptr, "��ʾ", "����ʧ��");
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
        ui->playBtn->setText("��ͣ");
    }
    else
    {
        ui->playBtn->setText("����");
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

        // ��ʾ���ļ���ҳ��
        ui->playWidget->setCurrentWidget(ui->openFilePage);
    }
    else
    {
        ui->playBtn->setEnabled(true);
        ui->stopBtn->setEnabled(true);
        ui->timeSlider->setEnabled(true);
        ui->volumnSlider->setEnabled(true);
        ui->muteBtn->setEnabled(true);

        // ��ʾ������Ƶ��ҳ��
        ui->playWidget->setCurrentWidget(ui->videoPage);
    }
}

void MainWindow::on_stopBtn_clicked()
{
    _player->stop();
}

void MainWindow::on_openFileBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(nullptr,
        "ѡ���ý���ļ�",
        "./res",
        "��ý���ļ���*.mp4 *.avi *.mkv *.mp3 *.aac)");
    if (filename.isEmpty()) return;

    // ��ʼ���Ŵ򿪵��ļ�
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
        ui->muteBtn->setText("����");
    }
    else
    {
        _player->setMute(true);
        ui->muteBtn->setText("����");
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
