#ifndef AUDIOREGION_H
#define AUDIOREGION_H

#include <QObject>
#include <QFileInfo>
#include <QJsonValue>

#include "fileloading.h"

#include "region.h"
class Track;

using namespace lab;

class AudioRegion : public Region
{
    Q_OBJECT
public:
    AudioRegion(AudioManager *_audioManager = nullptr, Track *parent = nullptr, QString uuid = "");

        void loadFile(QString fileName);
        void schedule();
        void cancelSchedule();

        void disconnectTrack();
        void setTrack(Track *_track);

        void switchContext(AudioContext *context);

        QString getLoadedFileName();
        QString preLoadedFile;

        void updateGridLength();

        bool isScheduled;

        std::shared_ptr<AudioBus> audioClipBus;
        std::shared_ptr<SampledAudioNode> audioClipNode;


    private:


        QString loadedFileName;

        void loadedFileCallBack();

        FileLoading *fileLoadingThread;

    };

#endif // AUDIOREGION_H
