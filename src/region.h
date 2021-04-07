#ifndef REGION_H
#define REGION_H

#include <QObject>

#include "audiomanager.h"
class AudioManager;

using namespace lab;

class Region : public QObject
{
    Q_OBJECT
public:
    Region(AudioManager *_audioManager = nullptr, Track *track = nullptr, QString uuid = "");
    ~Region();
    virtual Track* getTrack();
    virtual AudioManager* getAudioManager();
        virtual void setTrack(Track *_track);

        virtual void disconnectTrack();
        virtual void connectTrack();

        virtual void setGridLocation(double time);
        virtual double getGridLocation();

        virtual void setGridLength(double value);
        virtual double getGridLength();

        virtual void schedule();

        virtual float getGain();
        virtual void setGain(float _gain);

        virtual QString getRegionName();
        virtual void setRegionName(QString _name);

        virtual std::shared_ptr<GainNode> getOutputNode();

        virtual QString getUUID();

    protected:

        Track *track;
        AudioManager *audioManager;

        std::shared_ptr<GainNode> outputNode;

        double gridLocation;
        double length;

        QString regionName;

        float gain;

        QString uuid;

};

#endif // REGION_H
