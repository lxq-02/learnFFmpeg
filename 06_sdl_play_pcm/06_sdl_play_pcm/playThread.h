#pragma once

#include <QThread>

class playThread  : public QThread
{
	Q_OBJECT

public:
	playThread(QObject *parent);
	~playThread();

	void run();
};
