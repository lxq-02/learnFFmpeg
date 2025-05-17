#pragma once

#include <QtWidgets/QWidget>
#include "ui_MainWindow.h"

#include <YuvPlayer.h>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();
    void on_stopButton_clicked();

    void on_PlayerStateChanged();
    void on_nextButton_clicked();

private:
    Ui::MainWindowClass *ui;
    YuvPlayer* _player = nullptr;
};
