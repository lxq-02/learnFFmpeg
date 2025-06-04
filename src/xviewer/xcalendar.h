#pragma once
#include <QCalendarWidget>
#include <QPainter>
#include <set>
#include <QDate>

class XCalendar :public QCalendarWidget
{
public:
	XCalendar(QWidget* parent = nullptr);
	~XCalendar() override = default;

	void paintCell(QPainter* painter, 
		const QRect& rect, 
		const QDate& date) const override;

	void AddDate(QDate d) { mdate_.insert(d); }
	void ClearDate() { mdate_.clear(); }

private:
	std::set<QDate> mdate_; // 存放有视频的日期
};

