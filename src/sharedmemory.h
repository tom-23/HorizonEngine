#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <QThread>

class SharedMemory : public QThread
{
public:
    SharedMemory();
    void run() override;
};

#endif // SHAREDMEMORY_H
