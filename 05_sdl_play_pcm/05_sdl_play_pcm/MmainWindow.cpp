#include "MmainWindow.h"
#include <SDL.h>
#include <qDebug>

MmainWindow::MmainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MmainWindowClass())
{
    ui->setupUi(this);

    connect(ui->playButton, &QPushButton::clicked, this, &MmainWindow::on_playButton_clicked);
}

MmainWindow::~MmainWindow()
{
    delete ui;
}

void MmainWindow::on_playButton_clicked()
{
    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}
