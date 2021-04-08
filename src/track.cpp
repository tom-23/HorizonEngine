#include "track.h"
#include "audioregion.h"

Track::Track(AudioManager *_audioManager, QString _uuid) : QObject(_audioManager)
{
    audioManager = _audioManager;
    logs::out(3, "Creating track");

    lab::AudioContext& ac = *audioManager->context.get();
    audioManager = _audioManager;
    logs::out(3, "Setting input node");

    trackInputNode = std::make_shared<GainNode>(ac);
    logs::out(3, "Setting output node");

    uuid = _uuid;

    trackOutputNode = std::make_shared<GainNode>(ac);
    pannerNode = std::make_shared<StereoPannerNode>(ac);
    Lanalyser = std::make_shared<AnalyserNode>(ac);
    Ranalyser = std::make_shared<AnalyserNode>(ac);

    Lanalyser->setSmoothingTimeConstant(8.0);
    Ranalyser->setSmoothingTimeConstant(8.0);

    channelSplitter = std::make_shared<ChannelSplitterNode>(ac, 2);
    channelMerger = std::make_shared<ChannelMergerNode>(ac, 2);


    trackInputNode->gain()->setValue(1.0f);
    trackOutputNode->gain()->setValue(1.0f);

    audioManager->context.get()->connect(trackOutputNode, trackInputNode);
    audioManager->context.get()->connect(pannerNode, trackOutputNode);
    audioManager->context.get()->connect(channelSplitter, pannerNode);


    audioManager->context.get()->connect(Lanalyser, channelSplitter, 0, 0);
    audioManager->context.get()->connect(Ranalyser, channelSplitter, 0, 1);

    audioManager->context.get()->connect(channelMerger, Lanalyser, 0, 0);
    audioManager->context.get()->connect(channelMerger, Ranalyser, 1, 0);


    audioManager->context->connect(audioManager->getOutputNode(), channelMerger);

    regionList = new std::vector<class Region *>;

    peakdB = -100;
    setMute(false);
    setGain(0.0);
    setPan(0.0);
}

Track::~Track() {

    for (auto r : *regionList) {
        delete r;
    }

    audioManager->context->disconnect(audioManager->getOutputNode(), trackOutputNode);
    audioManager->context->disconnect(trackInputNode, trackOutputNode);
}


AudioRegion* Track::addAudioRegion(QString regionUUID) {

    AudioRegion *audioRegion = new AudioRegion(audioManager, this, regionUUID);
    regionList->push_back(audioRegion);
    return audioRegion;
}

void Track::setRegion(Region *_region) {
    regionList->insert(regionList->end(), _region);
}

void Track::removeRegion(Region *_region) {

    regionList->erase(regionList->begin() + getIndexOfRegion(_region));
    trackInputNode->uninitialize();

    Lanalyser->uninitialize();
    Ranalyser->uninitialize();

    audioManager->context->disconnect(trackInputNode, _region->getOutputNode());
    trackInputNode->initialize();

    Lanalyser->initialize();
    Ranalyser->initialize();
}

AudioManager* Track::getAudioManager() {
    return audioManager;
}

int Track::getIndexOfRegion(Region *region) {
    auto iterator = std::find(regionList->begin(), regionList->end(), region);
    regionList->begin();
    if (iterator != regionList->end()) {
       return std::distance(regionList->begin(), iterator);
    } else {
        return -1;
    }

}

std::shared_ptr<GainNode> Track::getTrackInputNode() {
    return trackInputNode;
}

std::shared_ptr<GainNode> Track::getTrackOutputNode() {
    return trackOutputNode;
}

void Track::scheduleAudioRegions() {
    for (int i = 0; i < int(regionList->size()); i++) {
        AudioRegion* audioRegion = dynamic_cast<AudioRegion*>(regionList->at(i));
        //double contextLocation = audioManager->gridTimeToContextSeconds(audioRegion->getGridLocation()) - audioManager->getCurrentRelativeTime();
        audioRegion->schedule();
        logs::out(3, "Scheduled a region...");
    }
}

void Track::scheduleNextAudioRegion() {
    AudioRegion *audioRegion = nullptr;
    for (int i = 0; i < int(regionList->size()); i++) {
        AudioRegion* current = dynamic_cast<AudioRegion*>(regionList->at(i));
        //qDebug() << "UUID:" << QString::fromStdString(current->getUUID());
        bool isBefore = current->getGridLocation() >= audioManager->getCurrentGridTime();
        bool isDuring = !isBefore && (((current->getGridLocation() + current->getGridLength()) > audioManager->getCurrentGridTime()));

        if (audioRegion == nullptr) {
            if (isBefore || isDuring) {
                audioRegion = current;
            }
        } else {

            //if () {
            //    if (audioRegion->getGridLocation() >= current->getGridLocation()) {
            //        audioRegion = current;
            //    }
            //}

            if (isDuring) {
                qDebug() << "is during scheduled";
                audioRegion = current;
            } else {
                if (current->getGridLocation() <= audioRegion->getGridLocation() && isBefore) {
                    qDebug() << "is before schedules";
                    audioRegion = current;
                } else {
                    qDebug() << "is not";
                    qDebug() << current->getGridLocation();
                    qDebug() << audioRegion->getGridLocation();
                }
            }

            qDebug() << "IS BEFORE?" << isBefore;
            qDebug() << "IS DURING?" << isDuring;
           // qDebug() << "IS AFTER?" << isAfter;
        }


    }

    if (audioRegion != nullptr) {
        qDebug() << "Scheduled UUID" << audioRegion->getUUID();
        //debug::out(3, "Scheduled a region...");
        audioRegion->schedule();
    } else {
        //debug::out(3, "No regions to schedule");
    }

}

void Track::scheduleNextBar() {
    int currentBar = floor(audioManager->getCurrentGridTime());
    int nextBlock = currentBar + 2;
    float currentGridLocation = audioManager->getCurrentGridTime();
    //AudioRegion *audioRegion = nullptr;
    for (int i = 0; i < int(regionList->size()); i++) {
        AudioRegion* audioRegion = dynamic_cast<AudioRegion*>(regionList->at(i));
        double gridLocation = audioRegion->getGridLocation();
        double endLocation = gridLocation + audioRegion->getGridLength();
        if (!audioRegion->isScheduled && (gridLocation <= nextBlock && gridLocation >= currentGridLocation)) {
            audioRegion->schedule();
            logs::out(3, "Scheduled a region...");
        }
        if (audioRegion->isScheduled && (gridLocation <= currentGridLocation && endLocation <= currentGridLocation)) {
            audioRegion->disconnectTrack();
            logs::out(3, "Canceled a region...");
        }
    }
}

void Track::cancelAudioRegions() {
    for (int i = 0; i < int(regionList->size()); i++) {
        AudioRegion* audioRegion = dynamic_cast<AudioRegion*>(regionList->at(i));
        audioRegion->disconnectTrack();
        logs::out(3, "Cancelling a region...");
    }
}

//void Track::removeRegion(int position) {
//    std::vector<class Region *>::iterator it = std::find(regionList->begin(), regionList->end(), _region);
//    if (it != regionList->end()) {
//
//        regionList->erase(std::distance(regionList->begin(), it));
//    }
//}

void Track::setGain(float _value) {
    gain = pow(10, (_value / 20));
    gainNonLog = _value;
    qDebug() << "Setting Gain" << gain;
    if (!mute) {
        trackOutputNode->gain()->setValue(gain);
    }

}

float Track::getGain() {

    return gainNonLog;
}

void Track::setPan(float _value) {
    pan = _value;
    pannerNode->pan()->setValue(_value);
}

float Track::getPan() {
    return pan;
}

void Track::setMute(bool _mute) {
    mute = _mute;
    if (mute == true) {
        trackOutputNode->gain()->setValue(0.0f);
    } else {
        trackOutputNode->gain()->setValue(gain);
    }
}

void Track::setSolo(bool _solo) {
    solo = _solo;
    if (solo == true) {

    }
}

bool Track::getMute() {
    return mute;
}

bool Track::getSolo() {
    return solo;
}


QPair<int, int> Track::getLMeterData() {

    std::vector<float> buffer(2048);

    Lanalyser->getFloatTimeDomainData(buffer);

    float sumOfSquares = 0.0;
    float peakInstantaneousPower = 0.0;

    for (int i = 0; i < (int)buffer.size(); i++) {
        sumOfSquares += pow(buffer[i], 2);
        float power = pow(buffer[i], 2);
        peakInstantaneousPower = std::max(power, peakInstantaneousPower);
    }
    float avgPowerDecibels = 10 * log10(sumOfSquares / buffer.size());
    float peakInstantaneousPowerDecibels = 10 * log10(peakInstantaneousPower);

    if (avgPowerDecibels >= peakdB) {
        peakdB = std::ceil(avgPowerDecibels * 100.0) / 100.0;

    }
    return {static_cast<int>(round(avgPowerDecibels)), static_cast<int>(round(peakInstantaneousPowerDecibels))};
}

QPair<int, int> Track::getRMeterData() {

    std::vector<float> buffer(2048);

    Ranalyser->getFloatTimeDomainData(buffer);

    float sumOfSquares = 0.0;
    float peakInstantaneousPower = 0.0;

    for (int i = 0; i < (int)buffer.size(); i++) {
        sumOfSquares += pow(buffer[i], 2);
        float power = pow(buffer[i], 2);
        peakInstantaneousPower = std::max(power, peakInstantaneousPower);
    }
    float avgPowerDecibels = 10 * log10(sumOfSquares / buffer.size());
    float peakInstantaneousPowerDecibels = 10 * log10(peakInstantaneousPower);

    if (avgPowerDecibels >= peakdB) {
        peakdB = std::ceil(avgPowerDecibels * 100.0) / 100.0;

    }

    return {static_cast<int>(round(avgPowerDecibels)), static_cast<int>(round(peakInstantaneousPowerDecibels))};
}

AudioRegion* Track::getAudioRegionByIndex(int index) {
    return dynamic_cast<AudioRegion*>(regionList->at(index));
}

int Track::audioRegionListCount() {
    return regionList->size();
}

QString Track::getUUID() {
    return uuid;
}

bool Track::isLSilent() {
    {
        ContextRenderLock r(audioManager->context.get(), "Horizon");
        return Lanalyser->inputsAreSilent(r);
    }
}

bool Track::isRSilent() {
    {
        ContextRenderLock r(audioManager->context.get(), "Horizon");
        return Ranalyser->inputsAreSilent(r);
    }
}
