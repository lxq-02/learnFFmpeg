#include "xcamera_widget.h"
#include <QStyleOption>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDebug>
#include <QListWidget>

XCameraWidget::XCameraWidget(QWidget* p)
	:QWidget(p)
{
	// 接收拖拽
	this->setAcceptDrops(true);
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent* ev)
{
	// 接收拖拽进入
	ev->acceptProposedAction();
}

void XCameraWidget::dropEvent(QDropEvent* ev)
{
	// 拿到url
	qDebug() << ev->source()->objectName();
	auto wid = (QListWidget*)ev->source();
	qDebug() << wid->currentRow();
}

void XCameraWidget::paintEvent(QPaintEvent* p)
{
	// 渲染样式表
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
