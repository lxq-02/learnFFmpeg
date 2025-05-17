#pragma once

#include <QThread>
#include <SDL.h>

class PlayThread  : public QThread
{
	Q_OBJECT

public:
	PlayThread(void *winId, QObject *parent = nullptr);
	~PlayThread();

private:
	void run();
	void* _winId;
};
