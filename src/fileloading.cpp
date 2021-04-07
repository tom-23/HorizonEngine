#include "fileloading.h"
#include "audioregion.h"

// Qt Shenanigans
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<AudioBus>)
Q_DECLARE_METATYPE(std::shared_ptr<SampledAudioNode>)
Q_DECLARE_METATYPE(std::vector<const float *>)

FileLoading::FileLoading()
{
    qRegisterMetaType<std::shared_ptr<AudioBus>>();
        qRegisterMetaType<std::shared_ptr<SampledAudioNode>>();
        qRegisterMetaType<std::vector<const float *>>();
}

void FileLoading::run() {
    logs::out(3, "Spawned file handling thread");
        logs::out(3, "Starting file loading...");
        audioRegion->audioClipBus = MakeBusFromSampleFile(fileName);

        logs::out(3, "Processing...");
        std::shared_ptr<lab::AudioContext> audioContext = audioRegion->getAudioManager()->context;

        audioRegion->audioClipNode = std::make_shared<SampledAudioNode>(*audioContext.get());
        {
            ContextRenderLock r(audioContext.get(), "Horizon");
            audioRegion->audioClipNode->setBus(r, audioRegion->audioClipBus);
        }

        logs::out(3, "Loaded audio, running callback function...");

        emit resultReady();
}

/* MakeBusFromSampleFile
    Loads the audio file into memory and passes it off the the audio region. Idk why I put the function
    here but its doing no harm so
*/
std::shared_ptr<AudioBus> FileLoading::MakeBusFromSampleFile(QString fileName) {

        std::shared_ptr<AudioBus> bus = MakeBusFromFile(fileName.toStdString(), false);

        if (!bus) {
            logs::out(1, "COULD NOT OPEN FILE: " + fileName);
            return nullptr;
        } else {
            logs::out(3, "Loaded audio file" + fileName);
        }
        return bus;
}

std::vector<const float *> FileLoading::getPeaks(std::shared_ptr<AudioBus> bus) {

    std::vector<const float *> channelSamples = {};

    for (int channelIdx = 0; channelIdx < (int)bus->numberOfChannels(); channelIdx++) {
         channelSamples.push_back(bus->channel(channelIdx)->data());
    }
    return channelSamples;
}
