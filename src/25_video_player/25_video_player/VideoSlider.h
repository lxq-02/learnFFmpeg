#pragma once

#include <QSlider>

class VideoSlider : public QSlider
{
	Q_OBJECT
public:
	explicit VideoSlider(QWidget* parent = nullptr);

signals:
		// ����¼�
	void clicked(VideoSlider* slider);

private:
	void mousePressEvent(QMouseEvent* ev) override;
};

