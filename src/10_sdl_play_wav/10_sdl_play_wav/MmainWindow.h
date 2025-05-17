#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MmainWindow.h"
#include "playThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MmainWindowClass; };
QT_END_NAMESPACE

class MmainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MmainWindow(QWidget *parent = nullptr);
    ~MmainWindow();

    void showVersion();
    void on_playButton_clicked();

private:
    Ui::MmainWindowClass *ui;
    playThread* _playThread = nullptr;
};
