#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "AudioThread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_audioButton_clicked();

private:
    Ui::MainWindowClass ui;
    AudioThread* _audioThread = nullptr;
};
