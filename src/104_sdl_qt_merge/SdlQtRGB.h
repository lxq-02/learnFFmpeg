#pragma once

#include <QWidget>
#include "ui_SdlQtRGB.h"
#include <iostream>
#include <SDL.h>

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

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;

	unsigned char* rgb;
};
