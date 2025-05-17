#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

    connect(ui->videoButton, &QPushButton::clicked, this, &MainWindow::on_videoButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_videoButton_clicked()
{
    PlayThread* thread = new PlayThread(this);
	thread->start();
}
