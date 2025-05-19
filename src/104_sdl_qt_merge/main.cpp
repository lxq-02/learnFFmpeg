#include <iostream>
#include <QApplication>
#include "SdlQtRGB.h"

// 将代码转移到timerEvent中
#undef main

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	SdlQtRGB window;
	window.show();

	return app.exec();
}
