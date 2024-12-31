#ifdef _MSC_VER
// MSVC�����������µĴ���
#define _T(str) QString::fromLocal8Bit(str)
#elif __GNUC__
// MinGW�����������µĴ���
#define _T(str) QString(str)
#endif

#include "MainWindow.h"
#include <qDebug>

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
    // ¼���߼�
    const char* fmtName = "dshow";
    AVInputFormat* fmt = av_find_input_format(fmtName);
    if (!fmt)
    {
        qDebug() << "��ȡ�����ʽ����ʧ��" << fmtName;
        return;
    }

    AVFormatContext* ctx = nullptr;
    ctx = avformat_alloc_context();

    // ʹ��QStringLiteral�������ַ���
    //QString deviceName = QStringLiteral("audio=���������� (Realtek(R) Audio)");
    QString deviceName = QStringLiteral("audio=��˷����� (2- Realtek(R) Audio)");

    // ֱ��ʹ��UTF-8����ת��Ϊconst char*��ȷ��������
    QByteArray deviceNameUtf8 = deviceName.toUtf8();
    const char* audio = deviceNameUtf8.constData();

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
}
