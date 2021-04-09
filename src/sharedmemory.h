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
public:
    SharedMemory();
    AudioManager *audioManager;
    void run() override;

    QTimer *updateTimer;

    QSystemSemaphore readSemaphore;
    QSystemSemaphore writeSemaphore;
    QSharedMemory sharedMemory;
};

#endif // SHAREDMEMORY_H
