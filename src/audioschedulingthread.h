#ifndef AUDIOSCHEDULINGTHREAD_H
#define AUDIOSCHEDULINGTHREAD_H

#include <QThread>
#include <QTimer>
#include <QCoreApplication>

class AudioManager;

class AudioSchedulingThread : public QThread
{
public:
    AudioSchedulingThread();
    AudioManager *audioManager;
    void run() override;
    void delay(int millisecondsToWait);
private:
    QTimer *delayTimer;
};

#endif // AUDIOSCHEDULINGTHREAD_H
