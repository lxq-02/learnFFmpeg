#include "MainWindow.h"
#include "FFmpegs.h"
#include <QtWidgets/QApplication>

extern "C" {
// �豸���API
#include <libavdevice/avdevice.h>
}

int main(int argc, char *argv[])
{
    WAVHeader header;
    header.riffChunkDataSize = 1767996;
    header.sampleRate = 44100;
    header.bitsPerSample = 16;
    header.numChannels = 2;
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign;
    header.dataChunkDataSize = 1767960;
    // pcmתwav�ļ�
    FFmpegs::pcm2wav(header, "D:/in.pcm", "D:/in.wav");
    // 1��ע���豸
    avdevice_register_all();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
