#include "audioschedulingthread.h"
#include "audiomanager.h"
#include "track.h"

AudioSchedulingThread::AudioSchedulingThread()
{

}

void AudioSchedulingThread::run() {
    delayTimer = new QTimer(this);
    connect(delayTimer, &QTimer::timeout, this, [=] () {
        if (audioManager->isPlaying) {
            for (int i = 0; i < int(audioManager->trackListCount()); i++) {
                    audioManager->getTrack(i)->scheduleNextBar();
            }
        }
    });

    delayTimer->start(100);
}
