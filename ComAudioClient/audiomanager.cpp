#include "audiomanager.h"

//Carson
AudioManager::AudioManager(QObject * par) {
    parent = par;

    win32CommunicationWorker = new Win32CommunicationWorker();
    win32CommunicationWorker->moveToThread(&win32CommunicationThread);
    connect(&win32CommunicationThread, &QThread::finished, win32CommunicationWorker, &QObject::deleteLater);
    connect(&win32CommunicationThread, SIGNAL(started()), win32CommunicationWorker, SLOT(doWork()));
    connect(win32CommunicationWorker, SIGNAL(playLoad()), this, SLOT(playRecord()));
    win32CommunicationThread.start();
}

void AudioManager::Init(QBuffer * buf, CircularBuffer * circ) {
    songState = Stopped;
    circularBuffer = circ;
    buffer = buf;
    qRegisterMetaType<wav_hdr>("wav_hdr");

    if (populateBufferWorker != NULL) {
        populateBufferWorker->deleteLater();
        delete populateBufferWorker;
        populateBufferWorker = NULL;
    }

    format.setSampleRate(44100);        // 48000
    format.setChannelCount(2);
    format.setSampleSize(16);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audioOutput = new QAudioOutput(format, parent);

    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

    audioOutput->setBufferSize(10000000);
    audioOutput->setVolume(volume);

    populateBufferWorker = new PopulateBufferWorker(circ, buf);
    populateBufferWorker->moveToThread(&populateBufferThread);
    connect(&populateBufferThread, SIGNAL(started()), populateBufferWorker, SLOT(doWork()));
    connect(&populateBufferThread, &QThread::finished, populateBufferWorker, &QObject::deleteLater);
    populateBufferThread.start();
}

void AudioManager::handleStateChanged(QAudio::State newState) {
    switch (newState) {
        case QAudio::IdleState:
            audioOutput->suspend();
            audioOutput->resume();
            break;
        default:
            break;
    }
}

AudioManager::~AudioManager() {
    delete audioOutput;
    delete circularBuffer;
    delete buffer;
    delete populateBufferWorker;
}

void AudioManager::loadSong(QFile * f) {
    readFileWorker = new ReadFileWorker(f, circularBuffer);
    readFileWorker->moveToThread(&readWorkerThread);
    connect(&readWorkerThread, &QThread::finished, readFileWorker, &QObject::deleteLater);
    connect(&readWorkerThread, SIGNAL(started()), readFileWorker, SLOT(doWork()));
    connect(readFileWorker, SIGNAL(gotWavHeader(wav_hdr)), this, SLOT(receivedWavHeader(wav_hdr)));
    circularBuffer->resetBuffer();
    buffer->seek(0);
    readWorkerThread.start();
}

void AudioManager::receivedWavHeader(wav_hdr wavHeader) {
    qDebug() << "Received Header";
    format.setSampleRate(wavHeader.SamplesPerSec);
    format.setChannelCount(wavHeader.NumOfChan);
    format.setSampleSize(wavHeader.bitsPerSample);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    audioOutput = new QAudioOutput(format, parent);
    audioOutput->setVolume(volume);
    songState = Playing;
    play();
}

void AudioManager::playRecord() {
    qDebug() << "Play Record";
    // Set up the desired format, for example:

    format.setSampleRate(15000);
    format.setChannelCount(1);
    format.setSampleSize(16);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    if (audioOutput != NULL) {
        delete audioOutput;
    }
    audioOutput = new QAudioOutput(format, parent);
    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    audioOutput->setBufferSize(10000000);
    audioOutput->setVolume(volume);
    songState = Playing;
    buffer->seek(0);

    SleepEx(500, true);
    play();
}

void AudioManager::pause() {
    audioOutput->suspend();
    songState = Paused;
}

void AudioManager::resume() {
    if (audioOutput->state() == QAudio::SuspendedState) {
        audioOutput->resume();
        songState = Playing;
    }
}

void AudioManager::stop() {
    audioOutput->stop();
    songState = Stopped;
}

void AudioManager::skip(float seconds) {
    //pause();
    int curPos = buffer->pos();
    int newPos = curPos + seconds * bytesPerSecond;
    if (newPos < 0)
        newPos = 0;
    if (newPos > buffer->size()-2)
        newPos = buffer->size()-2;
    buffer->seek(newPos);
    //resume();
}

QIODevice * AudioManager::play() {
    device = buffer;
    audioOutput->start(buffer);
    return buffer;
}

void AudioManager::setVolume(float vol) {
    volume = vol;
    audioOutput->setVolume(volume);
}
