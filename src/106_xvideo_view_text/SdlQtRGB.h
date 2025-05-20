#pragma once

#include <QWidget>
#include "ui_SdlQtRGB.h"
#include <iostream>
#include <SDL.h>
#include <QFile>

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

private:
	Ui::SdlQtRGBClass ui;

	QFile _file;
	int _timerId;

	unsigned char* rgb;

	int sdl_width;
	int sdl_height;
};
