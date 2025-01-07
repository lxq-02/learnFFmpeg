#include "MainWindow.h"
#include <QTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // ��ʼ��ʱ��
    onTimeChanged(0);
    
    connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::onTimeChanged(unsigned long long ms)
{
    QTime time(0, 0, 0, 0);
    QString text = time.addMSecs(ms).toString("mm::ss.z");
    ui.timeLabel->setText(text.left(7));
}

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread)   // ����ˡ���ʼ¼����
    {
        // ��ʼ�߳�
        _audioThread = new AudioThread(this);
        _audioThread->start();

        connect(_audioThread, &AudioThread::timechanged, this, &MainWindow::onTimeChanged);

        connect(_audioThread, &AudioThread::finished,
            [this]()    // �߳̽���
            {
                _audioThread = nullptr;
                ui.audioButton->setText(QStringLiteral("��ʼ¼��"));
            });

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
