#pragma once

#include <QtWidgets/QWidget>
#include "ui_MainWindow.h"
#include "VideoThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void on_videoButton_clicked();

private:
    Ui::MainWindowClass *ui;
    VideoThread* videoThread = nullptr;
};
