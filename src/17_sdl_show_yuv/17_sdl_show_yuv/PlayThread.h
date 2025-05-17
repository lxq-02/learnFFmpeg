#pragma once

#include <QThread>
#include <SDL.h>

class PlayThread  : public QThread
{
	Q_OBJECT

public:
	PlayThread(QObject *parent);
	~PlayThread();

private:
	void run();
};
