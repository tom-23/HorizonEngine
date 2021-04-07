#include "region.h"
#include "track.h"

Region::Region(AudioManager *_audioManager, Track *_track, QString _uuid) : QObject(_track)
{
    // here we assign some default variables
    audioManager = _audioManager;
    uuid = _uuid;
    track = _track;
    // create a new gain node. We will use this as the output gain.
    lab::AudioContext& ac = *track->getAudioManager()->context.get();
    outputNode = std::make_shared<GainNode>(ac);
    setGain(1.0f);
    // connect from outputNode -> trackInput node.
    track->getAudioManager()->context->connect(track->getTrackInputNode(), outputNode);
    // default to track position 1 and selected.
    gridLocation = 1;
}

Region::~Region() {
    track->getAudioManager()->context->disconnect(track->getTrackOutputNode(), outputNode);
}

Track* Region::getTrack() {
    return track;
}

AudioManager* Region::getAudioManager() {
    return audioManager;
}

void Region::disconnectTrack() {
    track->getAudioManager()->context.get()->disconnect(track->getTrackInputNode(), outputNode);
    logs::out(3, "Disconnected from track");
}

void Region::connectTrack() {
    track->getAudioManager()->context.get()->connect(track->getTrackInputNode(), outputNode);
    logs::out(3, "Connected to track");
}

void Region::setTrack(Track *_track) {
    track->getAudioManager()->context.get()->connect(_track->getTrackInputNode(), outputNode);
    logs::out(3, "Connected to track");
    setGain(gain);
    track = _track;
}

double Region::getGridLocation() {
    return gridLocation;
}

void Region::setGridLocation(double time) {
    gridLocation = time;
    logs::out(3, "Grid location just set!");
}

double Region::getGridLength() {
    return length;
}

void Region::setGridLength(double value) {
    length = value;
}

void Region::schedule() {

}

void Region::setGain(float _gain) {
    gain = _gain;
    outputNode->gain()->setValue(_gain);
}

float Region::getGain() {
    return gain;
}

std::shared_ptr<GainNode> Region::getOutputNode() {
    return outputNode;
}

QString Region::getRegionName() {
    return regionName;
}

void Region::setRegionName(QString _name) {
    regionName = _name;
}

QString Region::getUUID() {
    return uuid;
}
