#pragma once

#include "videoplayer.h"
#include <QWidget>
#include <QImage>

/*
œ‘ æ£®‰÷»æ£© ”∆µ
*/
class VideoWidget : public QWidget
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget* parent = nullptr);
	~VideoWidget();

public slots:
	void onPlayerFrameDecoded(VideoPlayer* player,
		uint8_t* data,
		VideoPlayer::VideoSwsSpec& spec);
	void onPlayerStateChanged(VideoPlayer* player);

private:
	QImage* _image = nullptr;
	QRect _rect;
	void paintEvent(QPaintEvent* event) override;

	void freeImage();

signals:
};

