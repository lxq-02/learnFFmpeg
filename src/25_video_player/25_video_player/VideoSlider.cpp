#include "VideoSlider.h"
#include <QMouseEvent>
#include <QStyle>

VideoSlider::VideoSlider(QWidget* parent)
	:QSlider(parent)
{
}

void VideoSlider::mousePressEvent(QMouseEvent* ev)
{
	// 根据点击位置的x值，计算出对应的value
	// valueRange = max -min
	// value = min + (x / width) * valueRange
	// int value = minimum() + (ev->pos().x() * 1.0 / width()) * (maximum() - minimum());
	// setValue(value);

	// 默认的函数，根据位置计算出对应的value
	int value = QStyle::sliderValueFromPosition(minimum(),
												maximum(),
												ev->pos().x(),
												width());
	setValue(value);

	// 调用基类的默认行为
	QSlider::mousePressEvent(ev);

	// 发出信号
	emit clicked(this);
}
