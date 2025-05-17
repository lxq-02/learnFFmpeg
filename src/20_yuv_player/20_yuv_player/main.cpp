#include "MainWindow.h"
#include <QtWidgets/QApplication>

#include <QDebug>
#include <SDL.h>

#undef main

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        qDebug() << "SDL_Init error: " << SDL_GetError();
        return 0;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int ret = a.exec();

    SDL_Quit();
    return ret;
}
