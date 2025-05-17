#include "MainWindow.h"
#include <QtWidgets/QApplication>

extern "C"
{
	#include "libavdevice/avdevice.h"
}

int main(int argc, char *argv[])
{
    // 1¡¢×¢²áÉè±¸
    avdevice_register_all();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
