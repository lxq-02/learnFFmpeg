#pragma once

#include <QtWidgets/QWidget>
#include "ui_MainWindow.h"

#include <QFile>
#include "videoplayer.h"
#include "videoslider.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onPlayerStateChanged(VideoPlayer* player);
    void onPlayerTimeChanged(VideoPlayer* player);
    void onPlayerInitFinished(VideoPlayer* player);
    void onPlayerPlayFailed(VideoPlayer* player);

    void onSliderClicked(VideoSlider* slider);

    void on_stopBtn_clicked();
    
    void on_openFileBtn_clicked();

    void on_timeSlider_valueChanged(int value);

    void on_volumnSlider_valueChanged(int value);

    void on_playBtn_clicked();

    void on_muteBtn_clicked();

private:
    Ui::MainWindowClass *ui;
    VideoPlayer* _player = nullptr;

    QString getTimeText(int value);
};
