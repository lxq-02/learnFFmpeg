#pragma once

#include <QThread>

class VideoThread  : public QThread
{
	Q_OBJECT

public:
	VideoThread(QObject *parent);
	~VideoThread();

private:
	void run();
};
