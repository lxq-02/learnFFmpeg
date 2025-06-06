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
	void SetSpeed();	// 控制播放速度
	void PlayPos();		// 控制播放进度 进度条松开
	void Pause();		// 播放或者暂停
	void Move();		// 进度条拖动

private:
	Ui::XPlayVideoClass ui;
	XPlayer player;
};
