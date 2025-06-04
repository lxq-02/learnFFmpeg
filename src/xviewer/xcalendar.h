#pragma once
#include <QCalendarWidget>
#include <QPainter>

class XCalendar :public QCalendarWidget
{
public:
	XCalendar(QWidget* parent = nullptr);
	~XCalendar() override = default;

	void paintCell(QPainter* painter, 
		const QRect& rect, 
		const QDate& date) const override;
};

