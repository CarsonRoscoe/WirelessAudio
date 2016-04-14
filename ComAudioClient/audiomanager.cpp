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
    audioOutput->setBufferSize(4096 * BUFFERSIZE);
    audioOutput->setVolume(volume);

    populateBufferWorker = new PopulateBufferWorker(circ, buf);
    populateBufferWorker->moveToThread(&populateBufferThread);
    connect(&populateBufferThread, SIGNAL(started()), populateBufferWorker, SLOT(doWork()));
    connect(&populateBufferThread, &QThread::finished, populateBufferWorker, &QObject::deleteLater);
    populateBufferThread.start();
}

AudioManager::~AudioManager() {
    delete audioOutput;
    delete circularBuffer;
    delete buffer;
    delete readFileWorker;
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

    QAudioFormat formatRecord;
    // Set up the desired format, for example:

    formatRecord.setSampleRate(15000);
    formatRecord.setChannelCount(1);
    formatRecord.setSampleSize(16);

    formatRecord.setCodec("audio/pcm");
    formatRecord.setByteOrder(QAudioFormat::LittleEndian);
    formatRecord.setSampleType(QAudioFormat::UnSignedInt);

    audioOutput = new QAudioOutput(formatRecord, parent);
    audioOutput->setBufferSize(4096 * BUFFERSIZE);
    audioOutput->setVolume(volume);
    songState = Playing;
    buffer->seek(0);
    play();
}

void AudioManager::pause() {
    audioOutput->suspend();
    songState = Paused;
}

void AudioManager::resume() {
    audioOutput->resume();
    songState = Playing;
}

void AudioManager::stop() {
    if (songState == Stopped)
        return;
    audioOutput->stop();
    file->close();
    delete audioOutput;
    songState = Stopped;
}
/*
void AudioManager::changeVolume(int vol) {
    audioOutput->setVolume(vol);
}
*/
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
    qDebug() << "Play entered";
    device = buffer;
    audioOutput->start(device);
    qDebug() << "Playing device, current buffered amount: " << buffer->size();
    return device;
}

void AudioManager::setVolume(float vol) {
    volume = vol;
    audioOutput->setVolume(volume);
}
