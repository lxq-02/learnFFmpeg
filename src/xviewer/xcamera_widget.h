#pragma once
#include <QWidget>

class XCameraWidget :public QWidget
{
	Q_OBJECT

public:
	XCameraWidget(QWidget* p = nullptr);
	
	// ÍÏ×§½øÈë
	void dragEnterEvent(QDragEnterEvent* ev) override;

	// ÍÏ×§ËÉ¿ª
	void dropEvent(QDropEvent* ev) override;

	// äÖÈ¾
	void paintEvent(QPaintEvent* p) override;
};

