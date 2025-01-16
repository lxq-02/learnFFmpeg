#pragma once

#include <QThread>

class PlayThread  : public QThread
{
	Q_OBJECT

public:
	PlayThread(QObject *parent);
	~PlayThread();

private:
	void run();
};
