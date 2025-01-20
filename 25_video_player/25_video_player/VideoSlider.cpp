#include "VideoSlider.h"
#include <QMouseEvent>
#include <QStyle>

VideoSlider::VideoSlider(QWidget* parent)
	:QSlider(parent)
{
}

void VideoSlider::mousePressEvent(QMouseEvent* ev)
{
	// ���ݵ��λ�õ�xֵ���������Ӧ��value
	// valueRange = max -min
	// value = min + (x / width) * valueRange
	// int value = minimum() + (ev->pos().x() * 1.0 / width()) * (maximum() - minimum());
	// setValue(value);

	int value = QStyle::sliderValueFromPosition(minimum(),
		maximum(),
		ev->pos().x(),
		width());
	setValue(value);

	QSlider::mousePressEvent(ev);

	// �����ź�
	emit clicked(this);
}
