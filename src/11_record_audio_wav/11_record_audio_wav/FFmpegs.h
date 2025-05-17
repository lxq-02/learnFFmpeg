#pragma once
#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3
#include <QObject>

// WAV�ļ�ͷ��44�ֽڣ�
typedef struct WAVHeader {
    // RIFF chunk��id
    uint8_t riffChunkId[4] = { 'R', 'I', 'F', 'F' };
    // RIFF chunk��data��С�����ļ��ܳ��ȼ�ȥ8�ֽ�
    uint32_t riffChunkDataSize;

    // "WAVE"
    uint8_t format[4] = { 'W', 'A', 'V', 'E' };

    /* fmt chunk */
    // fmt chunk��id
    uint8_t fmtChunkId[4] = { 'f', 'm', 't', ' ' };
    // fmt chunk��data��С���洢PCM����ʱ����16
    uint32_t fmtChunkDataSize = 16;
    // ��Ƶ���룬1��ʾPCM��3��ʾFloating Point
    uint16_t audioFormat = AUDIO_FORMAT_PCM;
    // ������
    uint16_t numChannels;
    // ������
    uint32_t sampleRate;
    // �ֽ��� = sampleRate * blockAlign
    uint32_t byteRate;
    // һ���������ֽ��� = bitsPerSample * numChannels >> 3
    uint16_t blockAlign;
    // λ���
    uint16_t bitsPerSample;

    /* data chunk */
    // data chunk��id
    uint8_t dataChunkId[4] = { 'd', 'a', 't', 'a' };
    // data chunk��data��С����Ƶ���ݵ��ܳ��ȣ����ļ��ܳ��ȼ�ȥ�ļ�ͷ�ĳ���(һ����44)
    uint32_t dataChunkDataSize;
} ;

class FFmpegs
{
public:
    static void pcm2wav(WAVHeader &header, const char* pcmFilename, const char* wavFilename);
};

