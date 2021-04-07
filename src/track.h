#ifndef TRACK_H
#define TRACK_H

#include <QObject>

#include <vector>
#include <memory>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <functional>
#include <math.h>
#include <iostream>

#include "audiomanager.h"
class AudioManager;
class AudioRegion;
class Region;

//class Region;

using namespace lab;

class Track : public QObject
{
    Q_OBJECT
public:
    Track(AudioManager *audioManager = nullptr, QString uuid = "");
    ~Track();
        AudioRegion* addAudioRegion(QString regionUUID);
        void setRegion(Region *_region);
        void removeRegion(Region *_region, Track *newTrack);

        AudioRegion* getAudioRegionByIndex(int index);
        int audioRegionListCount();

        void removeRegion(Region *_region);

        void scheduleAudioRegions();
        void cancelAudioRegions();

        std::shared_ptr<GainNode> getTrackInputNode();
        std::shared_ptr<GainNode> getTrackOutputNode();

        AudioManager* getAudioManager();

        QString getUUID();

        void setMute(bool _mute);
        void setSolo(bool _solo);

        bool getMute();
        bool getSolo();

        void setGain(float _value);
        float getGain();

        void setPan(float _value);
        float getPan();

        std::vector<int> getLMeterData();
        std::vector<int> getRMeterData();

        float peakdB;

        bool isLSilent();
        bool isRSilent();

        void scheduleNextAudioRegion();
        void scheduleNextBar();

    private:

        std::vector<class Region *> *regionList;

        QString uuid;

        std::shared_ptr<GainNode> trackInputNode;
        std::shared_ptr<GainNode> trackOutputNode;

        std::shared_ptr<AnalyserNode> Lanalyser;
        std::shared_ptr<AnalyserNode> Ranalyser;

        std::shared_ptr<ChannelSplitterNode> channelSplitter;
        std::shared_ptr<ChannelMergerNode> channelMerger;

        std::shared_ptr<StereoPannerNode> pannerNode;

        AudioManager *audioManager;

        bool mute = false;
        bool solo;

        float gain = 0.0f;
        float gainNonLog = 0.0f;
        float pan = 0.0f;

        int getIndexOfRegion(Region *region);

};

#endif // TRACK_H
