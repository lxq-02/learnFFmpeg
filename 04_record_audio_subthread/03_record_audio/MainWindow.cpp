#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread)   // ����ˡ���ʼ¼����
    {
        // ��ʼ�߳�
        _audioThread = new AudioThread(this);
        _audioThread->start();

        // �����߳�����
        ui.audioButton->setText(QStringLiteral("����¼��"));
    }
    else   // ����ʡ�����¼����
    {
        // �����߳�
        //_audioThread->setStop(true);
        _audioThread->requestInterruption();
        _audioThread = nullptr;

        // �����߳�����
        ui.audioButton->setText(QStringLiteral("��ʼ¼��"));
    }



    
}