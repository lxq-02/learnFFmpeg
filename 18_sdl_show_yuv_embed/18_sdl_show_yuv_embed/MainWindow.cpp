#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

    _widget = new QWidget(this);
    _widget->setGeometry(200, 50, 512, 512);

    connect(ui->videoButton, &QPushButton::clicked, this, &MainWindow::on_videoButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_videoButton_clicked()
{
    PlayThread* thread = new PlayThread((void*)_widget->winId(), this);
	thread->start();
}
