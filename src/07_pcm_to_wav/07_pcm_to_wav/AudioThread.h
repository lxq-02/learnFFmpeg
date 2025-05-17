#pragma once

#include <QThread>

class AudioThread  : public QThread
{
	Q_OBJECT
private:
	void run() override;
	bool _stop = false;

public:
	AudioThread(QObject *parent);
	~AudioThread();

	void setStop(bool);
};
