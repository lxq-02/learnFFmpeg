#include "YuvPlayer.h"
#include <vector>
#include <QPainter>
#include <QDebug>
#include "FFmpegs.h"

extern "C"
{
#include <libavutil/imgutils.h>
}

YuvPlayer::YuvPlayer(QWidget* parent)
	:QWidget(parent)
{
	// ���ñ���ɫ
	setAttribute(Qt::WA_StyledBackground);
	setStyleSheet("background: black");
}

YuvPlayer::~YuvPlayer()
{
	closeFile();
	freeCurrentImage();
	stopTimer();
}

void YuvPlayer::play()
{
	if (_state == YuvPlayer::Playing) return;

	// ״̬�����ǣ���ͣ��ֹͣ���������
	_timerId = startTimer(_interval);
	setState(YuvPlayer::Playing);
}

void YuvPlayer::pause()
{
	if (_state != YuvPlayer::Playing) return;

	// ״̬�����ǣ����ڲ���

	// ֹͣ��ʱ��
	stopTimer();

	// �ı�״̬
	setState(Paused);
}

void YuvPlayer::stop()
{
	if (_state == YuvPlayer::Stopped) return;

	// ״̬�����ǣ����ڲ��š���ͣ���������
	
	// ֹͣ��ʱ��
	stopTimer();

	// �ͷ�ͼƬ
	freeCurrentImage();

	// ˢ��
	update();

	// �ı�״̬
	setState(Stopped);
}

bool YuvPlayer::isPlaying()
{
	return _state == YuvPlayer::Playing;
}

void YuvPlayer::setYUV(Yuv& yuv)
{
	_yuv = yuv;

	// �ر���һ���ļ�
	closeFile();

	qDebug() << QStringLiteral("��") << yuv.filename;

	// ���ļ�
	_file = new QFile(yuv.filename);
	if (!_file->open(QFile::ReadOnly))
	{
		qDebug() << "file open error" << yuv.filename;
	}

	// ˢ֡��ʱ����
	_interval = 1000 / yuv.fps;

	// һ֡ͼƬ�Ĵ�С
	_imgSize = av_image_get_buffer_size(yuv.pixelFormat,
											yuv.width,
											yuv.height,
											1);

	// ����ĳߴ�
	int w = width();
	int h = height();

	// ����rect
	int dx = 0;
	int dy = 0;
	int dw = yuv.width;
	int dh = yuv.height;

	// ����Ŀ��ߴ�
	if (dw > w || dh > h) // ����
	{
		if (dw * h > w * dh) // ��Ƶ�Ŀ�߱ȴ��ڴ��ڵĿ�߱�
		{
			dh = w * dh / dw;
			dw = w;
		}
		else
		{
			dw = h * dw / dh;
			dh = h;
		}
	}

	// ����
	dx = (w - dw) >> 1;
	dy = (h - dh) >> 1;

	_dstRect = QRect(dx, dy, dw, dh);
	qDebug() << QStringLiteral("��Ƶ�ľ��ο�") << dx << dy << dw << dh;
}

// �������Ҫ�ػ��ʱ�򣬾ͻ�����������
// ��Ҫ����ʲô���ݣ��������������ʵ��
void YuvPlayer::paintEvent(QPaintEvent* event)
{
	if (!_currentImage) return;

	// ��ͼƬ���Ƶ���ǰ�����
	QPainter(this).drawImage(_dstRect, *_currentImage);
}

void YuvPlayer::timerEvent(QTimerEvent* event)
{
	std::vector<char> data(_imgSize);
	if (_file->read(data.data(), _imgSize) > 0)
	{
		RawVideoFrame in = {
			data.data(),
			_yuv.width,
			_yuv.height,
			_yuv.pixelFormat
		};
		RawVideoFrame out = {
			nullptr,
			_yuv.width >> 4 << 4,
			_yuv.height >> 4 << 4,
			AV_PIX_FMT_RGB24
		};
		FFmpegs::convertRawVideo(in, out);
		_currentImage = new QImage((uchar *)out.pixels,
			out.width, out.height, QImage::Format_RGB888);

		// ˢ��
		update();
	}
	else
	{
		// �ļ������Ѿ���ȡ���
		// ֹͣ��ʱ��
		stopTimer();

		// �����������
		setState(Finished);
	}
}

void YuvPlayer::closeFile()
{
	if (!_file) return;

	_file->close();
	delete _file;
	_file = nullptr;
}

void YuvPlayer::stopTimer()
{
	if (_timerId == 0) return;

	killTimer(_timerId);
	_timerId = 0;
}

void YuvPlayer::freeCurrentImage()
{
	if (!_currentImage) return;
	free(_currentImage->bits());
	delete _currentImage;
	_currentImage = nullptr;
}

void YuvPlayer::setState(State state)
{
	if (state == _state) return;

	if (state == Stopped || state == Finished)
	{
		// ���ļ���ȡָ��ص��ļ��ײ�
		_file->seek(0);
	}

	_state = state;
	emit stateChanged();
}

YuvPlayer::State YuvPlayer::getState()
{
	return _state;
}