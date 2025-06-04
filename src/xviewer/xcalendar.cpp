#include "xcalendar.h"

XCalendar::XCalendar(QWidget* parent)
	: QCalendarWidget(parent)
{
}

void XCalendar::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
	// 有视频的日期特殊显示
	// 测试日期 4号
	if (date.day() != 4)
	{
		QCalendarWidget::paintCell(painter, rect, date);
		return;
	}


	auto font = painter->font();
	// 设置字体
	font.setPointSize(30);

	// 选中状态刷背景色
	if (date == selectedDate())
	{
		painter->setBrush(QColor(118, 178, 224)); // 刷子颜色
		painter->drawRect(rect);					// 绘制背景
	}
	painter->setFont(font);
	painter->setPen(QColor(255, 0, 0)); // 设置字体颜色为红色
	painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

}
