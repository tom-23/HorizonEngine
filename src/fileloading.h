#ifndef FILELOADING_H
#define FILELOADING_H

#include <QThread>
#include "LabSound/LabSound.h"
//#include "audiomanager.h"

class AudioManager;
class AudioRegion;
//class AudioBus;

using namespace lab;

class FileLoading : public QThread
{
    Q_OBJECT
public:
    FileLoading();
    void run() override;
    QList<AudioRegion *> *audioRegionQueue;
    std::vector<const float *> peaks;
signals:
    void resultReady();
private:
    std::shared_ptr<AudioBus> MakeBusFromSampleFile(QString fileName);
    std::vector<const float *> getPeaks(std::shared_ptr<AudioBus> bus);
};

#endif // FILELOADING_H
