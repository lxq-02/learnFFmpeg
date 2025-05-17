#include "FFmpegs.h"
#include <QDebug>
#include <QFile>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#define ERR_BUF \
	char errbuf[1024]; \
	av_strerror(ret, errbuf, sizeof(errbuf));

#define END(func) \
	if (ret < 0) \
	{ \
		ERR_BUF; \
		qDebug() << #func << "error:" << errbuf; \
		goto end; \
	}

FFmpegs::FFmpegs()
{
}

void FFmpegs::convertRawVideo(RawVideoFrame in, RawVideoFrame out)
{
	// ������
	SwsContext *ctx = nullptr;
	// ���롢�����������ִ��ÿһ��ƽ������ݣ�
	uint8_t* inData[4], *outData[4];
	// ÿһ��ƽ��Ĵ�С
	int inStride[4], outStride[4];
	// ÿһ֡ͼƬ�Ĵ�С
	int inFrameSize, outFrameSize;
	// ���ؽ��
	int ret = 0;

	// ����������
	ctx = sws_getContext(in.width, in.height, in.format,
						out.width, out.height, out.format,
						SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!ctx)
	{
		qDebug() << "sws_getContext error";
		goto end;
	}

	// ���뻺����
	ret = av_image_alloc(inData, inStride,
		in.width, in.height, in.format, 1);
	END(av_image_alloc);

	// ���������
	ret = av_image_alloc(outData, outStride,
		out.width, out.height, out.format, 1);
	END(av_image_alloc);

	// ����ÿһ֡ͼƬ�Ĵ�С
	inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
	outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

	// ������������
	memcpy(inData[0], in.pixels, inFrameSize);
	
	// ת��
	sws_scale(ctx,
		inData, inStride, 0, in.height,
		outData, outStride);

	// д������ļ�ȥ
	out.pixels = (char*)malloc(outFrameSize);
	memcpy(out.pixels, outData[0], outFrameSize);

end:
	av_freep(&inData[0]);
	av_freep(&outData[0]);
	sws_freeContext(ctx);

}

// yuv420p
//    inData[0] = (uint8_t *) malloc(in.frameSize);
//    inData[1] = inData[0] + ����Y�Ĵ�С;
//    inData[2] = inData[0] + ����Y�Ĵ�С + ����U�Ĵ�С;

//    inStrides[0] = 640; // Y
//    inStrides[1] = 320; // U
//    inStrides[2] = 320; // V
void FFmpegs::convertRawVideo(RawVidelFile in, RawVidelFile out)
{
	// yuv420p -> yuv444p
	
	// ������
	SwsContext* ctx = nullptr;
	// ���롢�����������ִ��ÿһ��ƽ������ݣ�
	uint8_t* inData[4], * outData[4];
	// ÿһ��ƽ��һ�еĴ�С
	int inStride[4], outStride[4];
	// ÿһ֡ͼƬ�Ĵ�С
	int inFrameSize, outFrameSize;
	// ���ؽ��
	int ret = 0;
	// ���е�����һ֡
	int frameIdx = 0;
	// �ļ�
	QFile inFile(in.filename), outFile(out.filename);


	// ����������
	ctx = sws_getContext(in.width, in.height, in.format,
							out.width, out.height, out.format,
							SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!ctx)
	{
		qDebug() << "sws_getContext error";
		goto end;
	}

	// ���뻺����
	ret = av_image_alloc(inData, inStride,
		in.width, in.height, in.format, 1);
	END(av_image_alloc);

	// ���������
	ret = av_image_alloc(outData, outStride,
		out.width, out.height, out.format, 1);
	END(av_image_alloc);

	// ���ļ�
	if (!inFile.open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << in.filename;
		goto end;
	}
	
	if (!outFile.open(QFile::WriteOnly))
	{
		qDebug() << "file open error" << out.filename;
		goto end;
	}

	// ����ÿһ֡ͼƬ�Ĵ�С
	inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
	outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

	// ����ÿһ֡��ת��
	while (inFile.read((char*)inData[0], inFrameSize) == inFrameSize)
	{
		// ת��
		sws_scale(ctx,
			inData, inStride, 0, in.height,
			outData, outStride);
		// д������ļ�ȥ
		outFile.write((char*)outData[0], outFrameSize);
		qDebug() << QStringLiteral("ת�����") << frameIdx << QStringLiteral("֡");
	}

end:
	inFile.close();
	outFile.close();
	av_freep(&inData[0]);
	av_freep(&outData[0]);
	sws_freeContext(ctx);
}
