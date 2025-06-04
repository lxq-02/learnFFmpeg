#pragma once
#include <QWidget>

class XCameraWidget :public QWidget
{
	Q_OBJECT

public:
	XCameraWidget(QWidget* p = nullptr);
	
	// ��ק����
	void dragEnterEvent(QDragEnterEvent* ev) override;

	// ��ק�ɿ�
	void dropEvent(QDropEvent* ev) override;

	// ��Ⱦ
	void paintEvent(QPaintEvent* p) override;
};

