#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"

AudioThread::AudioThread(QObject* parent) : QThread(parent) 
{
    // ���������߳̽���ʱ��finished�����͵���deleteLater�����ڴ�
    connect(this, &AudioThread::finished,
        this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() 
{
    // �Ͽ����е�����
    disconnect();
    // �ڴ����֮ǰ�����������߳�
    requestInterruption();
    // ��ȫ�˳�
    quit();
    wait();
    qDebug() << this << "�������ڴ汻���գ�";
}

void AudioThread::run() 
{
    VideoEncodeSpec in;
    in.filename = "F:/res/dragon_ball.yuv";
    in.width = 640;
    in.height = 480;
    in.fps = 30;
    in.pixFmt = AV_PIX_FMT_YUV420P;

    FFmpegs::h264Encode(in, "F:/res/dragon_ball.h264");
}