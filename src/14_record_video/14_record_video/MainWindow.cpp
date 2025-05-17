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
    if (!videoThread)   // ����˿�ʼ¼��Ƶ
    {
        // �����߳�
		videoThread = new VideoThread(this);
		videoThread->start();

        connect(videoThread, &VideoThread::finished, [this]()
            {
                // �߳̽���
                videoThread = nullptr;
                ui->videoButton->setText(QStringLiteral("��ʼ¼��Ƶ"));
            });

        // ���ð�ť����
        ui->videoButton->setText(QStringLiteral("����¼��Ƶ"));
    }
    else // ����ˡ�����¼��Ƶ"
    {
        // �����߳�
        videoThread->requestInterruption();
        videoThread = nullptr;

        // ���ð�ť����
        ui->videoButton->setText(QStringLiteral("��ʼ¼��Ƶ"));
    }
}
