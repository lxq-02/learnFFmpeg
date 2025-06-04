#pragma once

#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QMenu>
#include "ui_xviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class XViewerClass; };
QT_END_NAMESPACE

class XViewer : public QWidget
{
    Q_OBJECT

public:
    XViewer(QWidget *parent = nullptr);
    ~XViewer();

public:
    // ����¼� �����϶�����
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

    // ���ڴ�С�����仯
    void resizeEvent(QResizeEvent* ev) override;
    // �Ҽ��˵�
    void contextMenuEvent(QContextMenuEvent* event) override;

    // ��ʾԤ����Ƶ����
    void View(int count);

    // ˢ���������б�
    void RefreshCams();

    // �༭�����
    void SetCam(int index);

    // ��ʱ����Ⱦ��Ƶ �ص�����
    void timerEvent(QTimerEvent* ev) override;

public slots:
    void MaxWindow();
    void NormalWindow();
    void View1();
    void View4();
    void View9();
    void View16();
    void AddCam(); // �������������
    void SetCam();
    void DelCam();

    void StartRecord(); // ��ʼȫ������ͷ¼��
	void StopRecord();  // ֹͣȫ������ͷ¼��
    void Preview();     // Ԥ��
    void Playback();    // �طŽ���

	void SelectCamera(QModelIndex index); // ѡ�������
	void SelectDate(QDate date);          // ѡ������
    void PlayVideo(QModelIndex index);    // ѡ��ʱ�䲥����Ƶ
private:
    Ui::XViewerClass *ui;
    QMenu left_menu_;
};
