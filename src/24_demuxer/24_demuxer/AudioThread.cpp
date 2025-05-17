#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"

extern "C" {
#include <libavutil/imgutils.h>
}

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
    VideoDecodeSpec out;
    out.filename = "F:/res/out.yuv";

    FFmpegs::h264Decode("F:/res/in.h264", out);

    qDebug() << out.width << out.height
        << out.fps << av_get_pix_fmt_name(out.pixFmt);
}