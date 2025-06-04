#pragma once

#include <QWidget>
#include <QTimerEvent>
#include <QCloseEvent>
#include "ui_xplayvideo.h"
#include "xdemux_task.h"
#include "xdecode_task.h"
#include "xvideo_view.h"

class XPlayVideo : public QWidget
{
	Q_OBJECT

public:
	XPlayVideo(QWidget *parent = nullptr);
	~XPlayVideo();

	bool Open(const char* url);

	void timerEvent(QTimerEvent* ev) override;

	void Close();
	void closeEvent(QCloseEvent* ev) override;

private:
	Ui::XPlayVideoClass ui;
	XDemuxTask demux_;
	XDecodeTask decode_;
	XVideoView* view_ = nullptr;
};
