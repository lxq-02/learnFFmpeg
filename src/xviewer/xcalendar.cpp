#include "xcalendar.h"

XCalendar::XCalendar(QWidget* parent)
	: QCalendarWidget(parent)
{
}

void XCalendar::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
	// ����Ƶ������������ʾ
	// �������� 4��
	if (date.day() != 4)
	{
		QCalendarWidget::paintCell(painter, rect, date);
		return;
	}


	auto font = painter->font();
	// ��������
	font.setPointSize(30);

	// ѡ��״̬ˢ����ɫ
	if (date == selectedDate())
	{
		painter->setBrush(QColor(118, 178, 224)); // ˢ����ɫ
		painter->drawRect(rect);					// ���Ʊ���
	}
	painter->setFont(font);
	painter->setPen(QColor(255, 0, 0)); // ����������ɫΪ��ɫ
	painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

}
