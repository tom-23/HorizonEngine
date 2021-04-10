#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <QThread>
#include <QTimer>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QBuffer>
#include <QDataStream>

class AudioManager;

class SharedMemory : public QThread
{
    Q_OBJECT
public:
    AudioManager *audioManager;
    void run() override;

    QTimer *updateTimer;

    void delay( int millisecondsToWait );

    void reCreateMemory(int size);

    int lastMemSize = 0;

    QSystemSemaphore *readSemaphore;
    QSystemSemaphore *writeSemaphore;
    QSharedMemory *sharedMemory;
};

#endif // SHAREDMEMORY_H
