#include "audiothread.h"

#include <QDebug>
#include "demuxer.h"

extern "C" {
#include <libavutil/imgutils.h>
}

AudioThread::AudioThread(QObject* parent) : QThread(parent) {
    // ���������߳̽���ʱ��finished�����͵���deleteLater�����ڴ�
    connect(this, &AudioThread::finished,
        this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // �Ͽ����е�����
    disconnect();
    // �ڴ����֮ǰ�����������߳�
    requestInterruption();
    // ��ȫ�˳�
    quit();
    wait();
    qDebug() << this << "�������ڴ汻���գ�";
}

void AudioThread::run() {
    AudioDecodeSpec aOut;
    aOut.filename = "F:/res/out.pcm";

    VideoDecodeSpec vOut;
    vOut.filename = "F:/res/out.yuv";

    Demuxer().demux("F:/res/in.mp4", aOut, vOut);

    qDebug() << aOut.sampleRate
        << av_get_channel_layout_nb_channels(aOut.chLayout)
        << av_get_sample_fmt_name(aOut.sampleFmt);

    qDebug() << vOut.width << vOut.height
        << vOut.fps << av_get_pix_fmt_name(vOut.pixFmt);
}