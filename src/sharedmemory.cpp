#include "sharedmemory.h"
#include "audiomanager.h"
#include "track.h"

void SharedMemory::delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void SharedMemory::run() {
    readSemaphore = new QSystemSemaphore("HorizonReadSemaphore");
    writeSemaphore = new QSystemSemaphore("HorizonWriteSemaphore");
    sharedMemory = new QSharedMemory("HorizonSharedData");


    if (!sharedMemory->create(1048576)) {
        qDebug() << sharedMemory->error();
        sharedMemory->attach();
        sharedMemory->detach();

        if (!sharedMemory->create(1048576)) {
            qDebug() << sharedMemory->error();
            logs::out(3, "Failed to create shared memory. Exiting thread... " + sharedMemory->errorString());
            return;
        }
    }

    logs::out(3, "Created shared memory. Begining update loop...");

    readSemaphore->release();

    while (true) {
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

        //qDebug() << "waiting for read semaphore";
        readSemaphore->acquire();
        //qDebug() << "RUNNING";

        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << horizonData;
        int size = buffer.size();

        //if (size != lastMemSize) {
        //    reCreateMemory(size);
        //}

        char *to = (char*)sharedMemory->data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory->size(), size));

        //sharedMemory->unlock();

        writeSemaphore->release();

        delay(50);
    }
}

void SharedMemory::reCreateMemory(int size) {
    char *to = (char*)sharedMemory->data();
    memset(to, 0, size);
    lastMemSize = size;
    logs::out(3, "Resized memory to " + QString::number(size));

    //sharedMemory->attach();
    //sharedMemory->detach();
    //if (!sharedMemory->create(size + 1024)) {
    //    qDebug() << "Unable to create shared memory segment." << sharedMemory->errorString();
    //    qDebug() << size + 1024;
//
    //}
}
