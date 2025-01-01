#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MmainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MmainWindowClass; };
QT_END_NAMESPACE

class MmainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MmainWindow(QWidget *parent = nullptr);
    ~MmainWindow();

    void on_playButton_clicked();

private:
    Ui::MmainWindowClass *ui;
};
