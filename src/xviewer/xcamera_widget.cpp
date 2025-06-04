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
	// ������ק
	this->setAcceptDrops(true);
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent* ev)
{
	// ������ק����
	ev->acceptProposedAction();
}

void XCameraWidget::dropEvent(QDropEvent* ev)
{
	// �õ�url
	qDebug() << ev->source()->objectName();
	auto wid = (QListWidget*)ev->source();
	qDebug() << wid->currentRow();
}

void XCameraWidget::paintEvent(QPaintEvent* p)
{
	// ��Ⱦ��ʽ��
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
