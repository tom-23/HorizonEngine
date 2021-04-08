#include "audiomanager.h"

#include "track.h"
#include "region.h"
#include "audioregion.h"
#include "server.h"

AudioManager::AudioManager(QObject *parent) : QObject(parent) {
    stopTime = 0.0;
    startTime = 0.0;
    isPlaying = false;
    currentGridTime = 1.0;
    scheduled = false;
    rendering = false;

    //initContext();

    eventTimer = new QTimer(this);


    trackList = new std::vector<class Track *>();

    audioSchedulingThread->audioManager = this;
    audioSchedulingThread->run();

    sharedMemory->audioManager = this;
    sharedMemory->run();
}

inline std::pair<AudioStreamConfig, AudioStreamConfig> GetDefaultAudioDeviceConfiguration(const bool with_input = false)
{
    AudioStreamConfig inputConfig;
    AudioStreamConfig outputConfig;

    const std::vector<AudioDeviceInfo> audioDevices = lab::MakeAudioDeviceList();
    const AudioDeviceIndex default_output_device = lab::GetDefaultOutputAudioDeviceIndex();
    const AudioDeviceIndex default_input_device = lab::GetDefaultInputAudioDeviceIndex();

    AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
    for (auto & info : audioDevices)
    {
        if (info.index == default_output_device.index) defaultOutputInfo = info;
        else if (info.index == default_input_device.index) defaultInputInfo = info;
    }

    if (defaultOutputInfo.index != -1)
    {
        outputConfig.device_index = defaultOutputInfo.index;
        outputConfig.desired_channels = std::min(uint32_t(2), defaultOutputInfo.num_output_channels);
        outputConfig.desired_samplerate = defaultOutputInfo.nominal_samplerate;
    }

    if (with_input)
    {
        if (defaultInputInfo.index != -1)
        {
            inputConfig.device_index = defaultInputInfo.index;
            inputConfig.desired_channels = std::min(uint32_t(1), defaultInputInfo.num_input_channels);
            inputConfig.desired_samplerate = defaultInputInfo.nominal_samplerate;
        }
        else
        {
            throw std::invalid_argument("the default audio input device was requested but none were found");
        }
    }

    logs::out(3, QString::fromStdString(defaultOutputInfo.identifier));

    return {inputConfig, outputConfig};
}


/* initContext
    Initalises a labsound real-time audio context
*/
void AudioManager::initContext() {
    const auto defaultAudioDeviceConfigurations = GetDefaultAudioDeviceConfiguration();
    context = lab::MakeRealtimeAudioContext(defaultAudioDeviceConfigurations.second, defaultAudioDeviceConfigurations.first);
    lab::AudioContext& ac = *context.get();

    outputNode = std::make_shared<GainNode>(ac);
    outputNode->gain()->setValue(1.0f);
    context->connect(context->device(), outputNode, 0 ,0);

}

void AudioManager::play() {
    if (isPlaying == false) {
        // taking the time of the context time as we will use this to schedule our regions and to know where we are during playback.
        startTime = context->currentTime();
        scheduleTracks();
        isPlaying = true;
        // if we are rendering audio offline, call the event timer to see a progress of file export.
        if (!rendering) {
            eventTimer->start();
        }
    }
}

void AudioManager::pause() {
    if (isPlaying == true) {
        isPlaying = false;
        cancelTrackPlayback();
        if (!rendering) {
            eventTimer->stop();
        }
        // see start time comments. It relates to that in some way.
        stopTime = getCurrentRelativeTime();
    }
}

void AudioManager::stop() {

    pause();
    // We put the playhead to the start when we hit the stop button. We also set the stop time to 0.0 too.
    stopTime = 0.0;
    currentGridTime = 1.0;
}

void AudioManager::eventLoop() {
    float relativeTime = (context->currentTime() - startTime) + stopTime;
    currentGridTime = ((relativeTime / beatLength) / division) + 1.0;
    //scheduleTracks();


    if (rendering == true) {
        //dialogs::ProgressDialog::updateValue(int(context->currentTime()));
    }
}

void AudioManager::setDivision(int _division) {
    division = _division;
    barLength = bpm * division;
}

void AudioManager::setBPM(double _beatsPerMinuet) {
    bpm = _beatsPerMinuet;
    beatLength = 60.00 / bpm;
    barLength = bpm * division;

    for (int t = 0; t < int(trackList->size()); ++t) {
        Track *track = trackList->at(t);
        for (int ar = 0; ar < track->audioRegionListCount(); ar++) {
            AudioRegion *audioRegion = track->getAudioRegionByIndex(ar);
            audioRegion->updateGridLength();
        }
    }
}

double AudioManager::getBPM() {
    return bpm;
}

float AudioManager::getCurrentGridTime() {
    if (context == nullptr) {
        return 1;
    }
    if (isPlaying) {
        float relativeTime = (context->currentTime() - startTime) + stopTime;
        currentGridTime = ((relativeTime / beatLength) / division) + 1.0;
    }
    return currentGridTime;
}

double AudioManager::gridTimeToContextSeconds(float _gridTime) {
    double secondsTime = ((_gridTime - 1.0) * beatLength) * division;
    return startTime + secondsTime;
}

double AudioManager::gridTimeToSeconds(float _gridTime) {
    double secondsTime = ((_gridTime) * beatLength) * division;
    return secondsTime;
}

float AudioManager::secondsToGridTime(double _seconds) {
    double gridTime = ((_seconds / beatLength) / division) + 1.0;
    return gridTime;
}

float AudioManager::getCurrentRelativeTime() {
    float relativeTime = (context->currentTime() - startTime) + stopTime;
    return relativeTime;
}

Track* AudioManager::addTrack(QString trackUUID) {
    logs::out(3, "Creating new track...");
    Track *newTrack = new Track(this, trackUUID);
    trackList->push_back(newTrack);
    return newTrack;
}

void AudioManager::removeTrack(Track *track) {
    logs::out(3, "Deleting track");

    auto iterator = std::find(trackList->begin(), trackList->end(), track);
    if (iterator != trackList->end()) {
        int index = std::distance(trackList->begin(), iterator);
        trackList->erase(trackList->begin() + index);
    }
    delete track;
}

std::shared_ptr<GainNode> AudioManager::getOutputNode() {
    return outputNode;
}


int AudioManager::trackListCount() {
    return trackList->size();
}

void AudioManager::scheduleTracks() {
    for (int i = 0; i < int(trackList->size()); i++) {
        trackList->at(i)->scheduleNextBar();
    }
}

void AudioManager::cancelTrackPlayback() {
    for (int i = 0; i < int(trackList->size()); i++) {
        trackList->at(i)->cancelAudioRegions();
        logs::out(3, "Cancelling track...");
    }
}

void AudioManager::setCurrentGridTime(float _value) {
    currentGridTime = _value;
}



void AudioManager::engageSolo() {
    soloEnabled = true;
    for (int i = 0; i < int(trackList->size()); i++) {
        if (trackList->at(i)->getSolo() == false) {
            trackList->at(i)->getTrackOutputNode()->gain()->setValue(0.0f);
        }

    }
}

void AudioManager::disengageSolo() {
    soloEnabled = false;
    for (int i = 0; i < int(trackList->size()); i++) {
        if (trackList->at(i)->getSolo() == false) {
            trackList->at(i)->getTrackOutputNode()->gain()->setValue(0.0f);
        }

    }
}

void AudioManager::clearAll() {
    for (auto p : *trackList) {
        delete p;
    }
    trackList->clear();
}

Track* AudioManager::getTrack(QString uuid) {
    for (int ti= 0; ti < this->trackListCount(); ti++) {
        Track *track = trackList->at(ti);
        if (track->getUUID() == uuid) {
            return track;
        }
    }
    return nullptr;
}

Track* AudioManager::getTrack(int index) {
    return trackList->at(index);
}

AudioRegion* AudioManager::getAudioRegion(QString uuid) {
    for (int ti= 0; ti < this->trackListCount(); ti++) {
        Track *track = trackList->at(ti);
        for (int ri = 0; ri < track->audioRegionListCount(); ri++) {
            AudioRegion *audioRegion = track->getAudioRegionByIndex(ri);
            if (audioRegion->getUUID() == uuid) {
                return audioRegion;
            }
        }
    }
    return nullptr;
}

void AudioManager::moveRegion(QString uuid, double gridLocation) {
    AudioRegion *audioRegion = getAudioRegion(uuid);
    if (this->isPlaying == true) {
        audioRegion->schedule();
    }
    audioRegion->setGridLocation(gridLocation);
}

void AudioManager::setTrackMute(QString uuid, bool mute) {
    Track *track = getTrack(uuid);
    track->setMute(mute);
}

void AudioManager::setTrackPan(QString uuid, float pan) {
    Track *track = getTrack(uuid);
    track->setPan(pan);
}

void AudioManager::setTrackGain(QString uuid, float gain) {
    Track *track = getTrack(uuid);
    track->setGain(gain);
}

void AudioManager::renderAudio(QObject *parent, QString fileName, int sampleRate, int channels) {

    qDebug() << "Rendering...";
    AudioStreamConfig offlineConfig;
    offlineConfig.device_index = 0;
    offlineConfig.desired_samplerate = sampleRate;
    offlineConfig.desired_channels = channels;

    qDebug() << "Config set";
    rendering = true;
    stop();
    eventTimer->start();
    qDebug() << "Started event timer";

    //FileRendering *fileRendering = new FileRendering(parent, [this] {
    //    rendering = false;
    //    stop();
    //    initContext();
    //    logs::out(3, "Finished Rendering Audio File!");
    //});
    //logs::out(3, "Rendering Audio File...");
    //fileRendering->operate(this, offlineConfig, fileName);
    //context.swap(offlineContext);
}

void AudioManager::loadAudioRegion(AudioRegion *audioRegion) {
    audioRegionLoadQueue->push_back(audioRegion);
    if (loadQueueThreads->size() == 0 && loadQueueThreads->size() < 5) {
        FileLoading *fileloading = new FileLoading();
        fileloading->audioRegionQueue = audioRegionLoadQueue;
        fileloading->start();
    }
}

void AudioManager::deSerialize(QJsonObject root) {

    setBPM(root.value("tempo").toDouble());

    for (int i = 0; i < root.value("tracks").toArray().size(); i++) {

        QJsonObject trackJSON = root.value("tracks").toArray().at(i).toObject();
        if (trackJSON.value("type") == "track") {
            logs::out(3, "Adding track");
            QString trackUuid;
            if (trackJSON.value("uuid").toString() == "") {
                trackUuid = QUuid::createUuid().toString();
            } else {
                trackUuid = trackJSON.value("uuid").toString();
            }
            Track *track = addTrack(trackUuid);

            for (int ar = 0; ar < trackJSON.value("audioRegions").toArray().size(); ar++) {
                QJsonObject audioRegionJSON = trackJSON.value("audioRegions").toArray().at(ar).toObject();

                if (audioRegionJSON.value("type").toString() == "audioRegion") {
                    logs::out(3, "Adding audio region");
                    QString regionUuid;
                    if (audioRegionJSON.value("uuid").toString() == "") {
                        regionUuid = QUuid::createUuid().toString();
                    } else {
                        regionUuid = audioRegionJSON.value("uuid").toString();
                    }
                    AudioRegion *audioRegion = track->addAudioRegion(regionUuid);
                    audioRegion->setGridLocation(std::stod(audioRegionJSON.value("gridLocation").toString().toStdString()));
                    QString tempDir = "";
                    if (audioRegionJSON.value("tempLocation").toBool()) { // if this file is being sent from the web...
                        tempDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/Horizon";
                    }
                    audioRegion->loadFile(tempDir + audioRegionJSON.value("filePath").toString());

                }
            }
            track->setGain(std::stof(trackJSON.value("gain").toString().toStdString()));
            track->setPan(std::stof(trackJSON.value("pan").toString().toStdString()));
            track->setMute(trackJSON.value("mute").toBool());
        }
    }
}
