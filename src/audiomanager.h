#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ratio>
#include <thread>
#include <math.h>
#include <ratio>
#include <QDebug>
#include <QJsonValue>
#include "LabSound/LabSound.h"


#include "logs.h"




class Track;
class Region;
class AudioRegion;

#include <QObject>
#include <QTimer>

using namespace lab;
using namespace std::chrono_literals;


class AudioManager : public QObject
{
    Q_OBJECT
public:
    AudioManager(QObject *parent = nullptr);
    void initContext();

    void play();
    void pause();
    void stop();

    bool isPlaying;

    void setDivision(int _division);
    void setBPM(double _beatsPerMinuet);
    double getBPM();

    float getCurrentGridTime();
    void setCurrentGridTime(float _value);

    double gridTimeToContextSeconds(float _gridTime);
    double gridTimeToSeconds(float _gridTime);
    float secondsToGridTime(double _seconds);
    float getCurrentRelativeTime();

    Track* addTrack(QString trackUUID);
    void removeTrack(Track *track);

    int trackListCount();
    void scheduleTracks();

    std::shared_ptr<GainNode> getOutputNode();


    std::shared_ptr<AudioContext> context;

    // uh?
    void engageSolo();
    void disengageSolo();

    bool soloEnabled;

    void clearAll();

    void moveRegion(QString uuid, double gridLocation);
    void setTrackMute(QString uuid, bool mute);
    void setTrackPan(QString uuid, float pan);
    void setTrackGain(QString uuid, float gain);

    Track* getTrack(QString uuid);
    AudioRegion* getAudioRegion(QString uuid);

    void renderAudio(QObject *parent, QString fileName, int sampleRate, int channels);



    void eventLoop();

private:

    float startTime;
    float stopTime;

    std::shared_ptr<GainNode> outputNode;

    std::vector<class Track *> *trackList;

    //Metronome *metronome;

    double bpm;
    double beatLength;
    double barLength;

    bool quitLoop;


    int division;
    /*
     * currentPos is:
     * barNumber.division
     * In the future, I could implement a time-based grid but that would take time.
     */
    int currentPos;
    double lookAhead;

    float currentGridTime;
    bool scheduled;

    void cancelTrackPlayback();

    //AudioSchedulingThread *audioSchedulingThread;

    bool rendering;

    QTimer *eventTimer;

signals:
    //void send_stat_message(QString message, QJsonValue value1);
    void send_stat_message(QString uuid, QJsonValue value1, QJsonValue value2);
};

#endif // AUDIOMANAGER_H
