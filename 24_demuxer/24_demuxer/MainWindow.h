#pragma once

#include <QtWidgets/QWidget>
#include "ui_MainWindow.h"

#include <QFile>
#include "audiothread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_audioButton_clicked();

private:
    Ui::MainWindowClass *ui;
    AudioThread* _audioThread = nullptr;
};
