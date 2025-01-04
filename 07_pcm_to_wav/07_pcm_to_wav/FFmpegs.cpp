#include "FFmpegs.h"
#include <QFile>
#include <QDebug>

void FFmpegs::pcm2wav(WAVHeader& header, const char* pcmFilename, const char* wavFilename)
{
    // ��pcm�ļ�
    QFile pcmFile(pcmFilename);
    if (!pcmFile.open(QIODevice::ReadOnly))
    {
        qDebug() << pcmFilename << "�ļ���ʧ��";
        return;
    }

    // ��wav�ļ�
    QFile wavFile(wavFilename);
    if (!wavFile.open(QIODevice::WriteOnly))
    {
        qDebug() << wavFilename << "�ļ���ʧ��";
        pcmFile.close();
        return;
    }

    // д��ͷ��
    wavFile.write((const char*) & header, sizeof(WAVHeader));

    // д��pcm����
    char buf[1024];
    int size;
    while ((size = pcmFile.read(buf, sizeof(buf))) > 0)
    {
        wavFile.write(buf, size);
    }

    // �ر��ļ�
    wavFile.close();
    pcmFile.close();
}
