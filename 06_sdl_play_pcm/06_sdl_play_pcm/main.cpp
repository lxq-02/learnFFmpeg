#include "MmainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MmainWindow w;
    w.show();
    return a.exec();
}
