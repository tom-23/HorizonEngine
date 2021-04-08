#include "audioregion.h"
#include "track.h"
#include "region.h"

AudioRegion::AudioRegion(AudioManager *_audioManager, Track *_track, QString _uuid) : Region(_audioManager, _track, _uuid) {

}

void AudioRegion::loadFile(QString fileName) {

    logs::out(3, "Begining file loading...");
    QFileInfo fileInfo(fileName);
    setRegionName(fileInfo.fileName());
    loadedFileName = fileName;
    audioManager->loadAudioRegion(this);
    return;
}

void AudioRegion::loadedFileCallBack() {

    track->getAudioManager()->context->connect(outputNode, audioClipNode);

    updateGridLength();
    logs::out(3, "Length calculated");

    isScheduled = false;
    logs::out(3, "Successfully Loaded File!");

    emit audioManager->send_stat_message("finishedAudioRegionLoad", uuid, length);

}

void AudioRegion::schedule() {
    float timeEnd = length + gridLocation;
    Region::connectTrack();
    audioClipNode->initialize();

    if (track->getAudioManager()->getCurrentGridTime() > gridLocation && track->getAudioManager()->getCurrentGridTime() < timeEnd) {
        logs::out(3, "Scheduled region during playhead");
        float playheadDiff = track->getAudioManager()->getCurrentGridTime() - gridLocation;
        float gttsPlayheadDiff = track->getAudioManager()->gridTimeToSeconds(playheadDiff);
        qDebug() << "GRID TIME" << gttsPlayheadDiff;
        audioClipNode->schedule(0.0, gttsPlayheadDiff);
        isScheduled = true;
        return;
    }

    if (track->getAudioManager()->getCurrentGridTime() <= gridLocation ) {
        //debug::out(3, "Scheduled region ahead of playhead");
        double timeToGo = (track->getAudioManager()->gridTimeToSeconds(gridLocation - track->getAudioManager()->getCurrentGridTime()));
        audioClipNode->schedule(timeToGo);
        isScheduled = true;
        return;
    }
}

void AudioRegion::cancelSchedule() {

    {
        ContextRenderLock r(track->getAudioManager()->context.get(), "Horizon");
        audioClipNode->reset(r);
    }

    isScheduled = false;
}

void AudioRegion::disconnectTrack() {
    cancelSchedule();
    Region::disconnectTrack();
}

void AudioRegion::setTrack(Track *_track) {


    {
        ContextRenderLock r(track->getAudioManager()->context.get(), "Horizon");
        audioClipNode->reset(r);

    }

    logs::out(3, "Switching Tracks...");

    _track->getAudioManager()->context->connect(_track->getTrackInputNode(), outputNode);



    audioClipNode->initialize();

    logs::out(3, "Connected to track");
    setGain(gain);

    track = _track;
}

QString AudioRegion::getLoadedFileName() {
    return loadedFileName;
}


void AudioRegion::switchContext(AudioContext *context) {

}


void AudioRegion::updateGridLength() {
    length = track->getAudioManager()->secondsToGridTime(audioClipNode->duration()) - 1;
}
