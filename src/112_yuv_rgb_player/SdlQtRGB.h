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

	// �̺߳���������ˢ����Ƶ
	void Main();

signals:
	void ViewS();	// �źź���������������б�
public slots:
	void View();	// ��ʾ�Ĳۺ���
	void Open1();
	void Open2();
	void Open(int i);

private:
	std::thread _th;
	Ui::SdlQtRGBClass ui;

	bool _is_exit = false;	// �����߳��˳�

	std::ifstream _file;
	int _timerId;

	unsigned char* rgb;

	int sdl_width;
	int sdl_height;
};
