
#include "MainWindow.h"
#include <QtWidgets/QApplication>

#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/*
����Ƶͬ����
1����Ƶͬ������Ƶ
2����Ƶͬ������Ƶ
*/

/*
1����ʵʱ��
����һ����Ƶ��ʱ����120�룬����120�������ʵʱ��
����һ����Ƶ���ŵ��˵�58�룬���е�58�������ʵʱ��

2��FFmpegʱ��
1>ʱ�����timestamp)��������int64_t
2>ʱ�����time base\unit������ʱ����ĵ�λ��������AVRational

3��FFmpegʱ�� �� ��ʵʱ���ת��
1> ��ʵʱ�� = ʱ��� * (ʱ����ķ��� / ʱ����ķ�ĸ)
2> ��ʵʱ�� = ʱ��� * av_q2d��ʱ�����
3> ʱ���= ��ʵʱ�� / (ʱ����ķ��� / ʱ����ķ�ĸ)
4> ʱ��� = ��ʵʱ�� / av_q2d��ʱ�����

*/