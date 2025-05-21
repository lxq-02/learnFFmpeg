#pragma once

#include <QWidget>
#include "ui_SdlQtRGB.h"
#include <iostream>
#include <SDL.h>
#include <QFile>
#include <thread>
#include <fstream>

using namespace std;

#pragma comment(lib, "SDL2.lib")

static int w = 800;
static int h = 600;

class SdlQtRGB : public QWidget
{
	Q_OBJECT

public:
	SdlQtRGB(QWidget *parent = nullptr);
	~SdlQtRGB();

	void timerEvent(QTimerEvent* ev) override;
	void resizeEvent(QResizeEvent* ev) override;

	// 线程函数，用于刷新视频
	void Main();

signals:
	void ViewS();	// 信号函数，将任务放入列表
public slots:
	void View();	// 显示的槽函数
	void Open1();
	void Open2();
	void Open(int i);

private:
	std::thread _th;
	Ui::SdlQtRGBClass ui;

	bool _is_exit = false;	// 处理线程退出

	std::ifstream _file;
	int _timerId;

	unsigned char* rgb;

	int sdl_width;
	int sdl_height;
};
