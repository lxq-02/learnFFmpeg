#include <iostream>
#include <QApplication>
#include "SdlQtRGB.h"

// ������ת�Ƶ�timerEvent��
#undef main

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	SdlQtRGB window;
	window.show();

	return app.exec();
}
