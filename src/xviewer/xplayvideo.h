#pragma once

#include <QDialog>
#include <QTimerEvent>
#include <QCloseEvent>
#include "ui_xplayvideo.h"
#include "xplayer.h"
class XPlayVideo : public QDialog
{
	Q_OBJECT

public:
	XPlayVideo(QDialog*parent = nullptr);
	~XPlayVideo();

	bool Open(const char* url);

	void timerEvent(QTimerEvent* ev) override;

	void Close();
	void closeEvent(QCloseEvent* ev) override;

public:
	void SetSpeed();

private:
	Ui::XPlayVideoClass ui;
	XPlayer player;
};
