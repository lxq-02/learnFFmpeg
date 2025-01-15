#include "MainWindow.h"
#include <qDebug>
#include <QFile>
#include <QThread>

extern "C" {
    // �豸���API
#include <libavdevice/avdevice.h>
    // ��ʽ���API
#include <libavformat/avformat.h>
    // �������API�����������
#include <libavutil/avutil.h>
    // �������API
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
    // ��ʽ����
    #define FMT_NAME "dshow"
    // PCM�ļ���
    #define FILENAME "D:/out.pcm"
#else
    #define FMT_NAME "avfoundation"
    #define FILENAME "/Users/mj/Destop/out.pcm"
#endif // Q_OS_WIN


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    qDebug() << "mainwindow" << QThread::currentThread();

    connect(ui.audioButton, &QPushButton::clicked, this, &MainWindow::on_audioButton_clicked);
}

MainWindow::~MainWindow()
{}

void MainWindow::on_audioButton_clicked()
{
    qDebug() << "on_audioButton_clicked" << QThread::currentThread();

    // ¼���߼�
    // ��ȡ�����ʽ����
    AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        qDebug() << "��ȡ�����ʽ����ʧ��" << FMT_NAME;
        return;
    }

    // ��ʽ�����ģ��������������Ĳ����豸��
    AVFormatContext* ctx = nullptr;

#ifdef Q_OS_WIN
    QString deviceName = QStringLiteral("audio=���������� (Realtek(R) Audio)");
    //QString deviceName = QStringLiteral("audio=��˷����� (2- Realtek(R) Audio)");
    QByteArray deviceNameUtf8 = deviceName.toUtf8();
    const char* audio = deviceNameUtf8.constData();
#else
    const char* audio = ":0";
#endif 
    // ���豸
    int ret = avformat_open_input(&ctx, audio, fmt, nullptr);
    if (ret < 0)
    {
        char errbuf[1024] = { 0 };
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << QStringLiteral("���豸ʧ��") << errbuf;
        return;
    }
    else
    {
        qDebug() << QStringLiteral("���豸�ɹ�");
    }

    // �ļ���
    QFile file(FILENAME);
    
    // ���ļ�
    // WriteOnly:ֻдģʽ������ļ������ڣ������ļ�������ļ����ڣ�����ļ�
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << FILENAME << QStringLiteral("�ļ���ʧ��");
        
        // �ر��豸
        avformat_close_input(&ctx);
        return;
    }

    // �ɼ��Ĵ���
    int count = 50;

    // ���ݰ�
    AVPacket pkt;
    // ���ϲɼ�����
    while (count-- > 0 && av_read_frame(ctx, &pkt) == 0)
    {
        // ������д���ļ�
        file.write((const char*)pkt.data, pkt.size);
    }

    // �ͷ���Դ
    // �ر��ļ�
    file.close();

    // �ر��豸
    avformat_close_input(&ctx);

    qDebug() << pkt.size;
}
