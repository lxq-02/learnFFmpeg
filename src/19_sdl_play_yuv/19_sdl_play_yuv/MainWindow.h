#pragma once

#include <QtWidgets/QWidget>
#include "ui_MainWindow.h"

#include <SDL.h>
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

    void on_videoButton_clicked();

private:
    Ui::MainWindowClass *ui;
    QWidget* _widget;
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;
    QFile _file;
    int _timerId;

    void timerEvent(QTimerEvent* event);
};
