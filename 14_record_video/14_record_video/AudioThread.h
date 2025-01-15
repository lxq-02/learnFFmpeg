#pragma once

#include <QThread>

class AudioThread  : public QThread
{
	Q_OBJECT
private:
	void run() override;

public:
	AudioThread(QObject *parent);
	~AudioThread();
};
