#include "sharedmemory.h"
#include "audiomanager.h"
#include "track.h"

SharedMemory::SharedMemory() :
    readSemaphore("HorizonReadSemaphore"),
    writeSemaphore("HorizonWriteSemaphore"),
    sharedMemory("HorizonSharedData")
{

}

void SharedMemory::run() {
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [=] () {
        QPair<double, QList<QList<int>>> horizonData;

        horizonData.first = audioManager->getCurrentGridTime();

        for (int i = 0; i < audioManager->trackListCount(); i++) {
            Track *track = audioManager->getTrack(i);

            QPair<int,int> lMeterData = track->getLMeterData();
            QPair<int,int> rMeterData = track->getRMeterData();

            QList<int> trackData;

            trackData.append(lMeterData.first);
            trackData.append(lMeterData.second);

            trackData.append(rMeterData.first);
            trackData.append(rMeterData.second);

            horizonData.second.append(trackData);
        }

        readSemaphore.acquire();

        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << horizonData;
        int size = buffer.size();

        sharedMemory.lock();

        if (!sharedMemory.create(size)) {
            qDebug() << "Unable to create shared memory segment.";
            return;
        }

        char *to = (char*)sharedMemory.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory.size(), size));

        sharedMemory.unlock();

        writeSemaphore.release();
    });

    updateTimer->start(10);
}
