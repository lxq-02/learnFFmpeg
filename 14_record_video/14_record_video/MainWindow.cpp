#include "MainWindow.h"
#include <QTime>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
    //connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
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

        connect(_audioThread, &AudioThread::finished,
            [this]()    // �߳̽���
            {
                _audioThread = nullptr;
                ui.audioButton->setText(QStringLiteral("��ʼ¼��Ƶ"));
            });

        // �����߳�����
        ui.audioButton->setText(QStringLiteral("����¼��Ƶ"));
    }
    else   // ����ʡ�����¼����
    {
        // �����߳�
        _audioThread->requestInterruption();
        _audioThread->wait(); // �ȴ��߳���ȫ�˳�
        delete _audioThread;  // �ֶ������̶߳���
        _audioThread = nullptr;

        // �����߳�����
        ui.audioButton->setText(QStringLiteral("��ʼ��Ƶ "));
    }
}
