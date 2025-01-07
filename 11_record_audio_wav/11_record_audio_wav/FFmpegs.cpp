#include "FFmpegs.h"
#include <QFile>
#include <QDebug>

void FFmpegs::pcm2wav(WAVHeader& header, const char* pcmFilename, const char* wavFilename)
{
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign;

    // ��pcm�ļ�
    QFile pcmFile(pcmFilename);
    if (!pcmFile.open(QIODevice::ReadOnly))
    {
        qDebug() << pcmFilename << "�ļ���ʧ��";
        return;
    }
    header.dataChunkDataSize = pcmFile.size();
    header.riffChunkDataSize = header.dataChunkDataSize + sizeof(WAVHeader) 
                                - sizeof(header.riffChunkId)
                                - sizeof(header.riffChunkDataSize);

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
