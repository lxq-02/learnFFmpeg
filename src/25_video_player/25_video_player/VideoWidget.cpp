#include "VideoWidget.h"
#include <QDebug>
#include <QPainter>
#include <QRect>

VideoWidget::VideoWidget(QWidget* parent)
	:QWidget(parent)
{
	// ���ñ���ɫ
	setAttribute(Qt::WA_StyledBackground);
	setStyleSheet("background: black;");
}

VideoWidget::~VideoWidget()
{
	freeImage();
}

void VideoWidget::onPlayerStateChanged(VideoPlayer* player)
{
	if (player->getState() != VideoPlayer::Stopped) return;

	freeImage();
	update();
}

void VideoWidget::onPlayerFrameDecoded(VideoPlayer* player,
	uint8_t* data,
	VideoPlayer::VideoSwsSpec& spec)
{
	if (player->getState() == VideoPlayer::Stopped) return;

	// �ͷ�֮ǰ��ͼƬ
	freeImage();

	// �����µ�ͼƬ
	if (data != nullptr)
	{
		_image = new QImage(data,
			spec.width, spec.height,
			QImage::Format_RGB888);

		// �������յĳߴ�
		// ����ĳߴ�
		int w = width();
		int h = height();

		// ����rect
		int dx = 0;
		int dy = 0;
		int dw = spec.width;
		int dh = spec.height;

		// ����Ŀ��ߴ�
		if (dw > w || dh > h) // ����
		{
			if (dw * h > w * dh) // ��Ƶ�Ŀ�߱� > �������Ŀ�߱�
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

		_rect = QRect(dx, dy, dw, dh);
	}

	update();
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
	if (!_image) return;

	// ��ͼƬ���Ƶ���ǰ�����
	QPainter(this).drawImage(_rect, *_image);
}

void VideoWidget::freeImage()
{
	if (_image)
	{
		av_free(_image->bits());
		delete _image;
		_image = nullptr;
	}
}
