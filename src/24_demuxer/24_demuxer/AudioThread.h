#pragma once
#include <QThread>

class AudioThread : public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit AudioThread(QObject* parent = nullptr);
    ~AudioThread();
signals:

};

