/*---------------------------------------------------------------------------------------------------------------------
-- CLASS FILE: audiomanager.cpp
--
-- PROGRAM: ComAudioClient
--
-- METHODS:
--    Constructor(QObject* parent)
--    ~Destructor()
--    void Init(QBuffer * buf, CircularBuffer * circ)
--    void handleStateChanged(QAudio::State newState)
--    void loadSong(QFile * f)
--    void receivedWavHeader(wav_hdr wavHeader)
--    void playRecord()
--    void pause()
--    void resume()
--    void stop()
--    void skip(float seconds)
--    QIODevice* play()
--    void setVolume(float volume)
--
-- DATE: April 1st 2016
--
-- REVISIONS: April 1st 2016: Created
--            April 13th 2016: Integrated
--            APril 14th, 2016: Commented
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- Handles playing/handling sound for the application and utilizes circuler buffers to do so.
---------------------------------------------------------------------------------------------------------------------*/
#include "audiomanager.h"

/*---------------------------------------------------------------------------------------
--	METHOD:         Constructor
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      AudioManager(QObject* pointer to the parent window)
--
--  RETURNS:        N/A
--
--	NOTES:
--  Basic constructor for our AudioManager class
---------------------------------------------------------------------------------------*/
AudioManager::AudioManager(QObject * par) {
    parent = par;

    win32CommunicationWorker = new Win32CommunicationWorker();
    win32CommunicationWorker->moveToThread(&win32CommunicationThread);
    connect(&win32CommunicationThread, &QThread::finished, win32CommunicationWorker, &QObject::deleteLater);
    connect(&win32CommunicationThread, SIGNAL(started()), win32CommunicationWorker, SLOT(doWork()));
    connect(win32CommunicationWorker, SIGNAL(playLoad()), this, SLOT(playRecord()));
    win32CommunicationThread.start();
}

/*---------------------------------------------------------------------------------------
--	METHOD:         Init
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void Init(QBuffer* QT buffer within the QAudioOutput class
--                            CircularBuffer* pointer to our circular buffer
--
--  RETURNS:        void
--
--	NOTES:
--  Reinitiates our class and resets all the data
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         handleStateChanged
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void handleStateChanged(QAudio::State new QIODevice state)
--
--  RETURNS:        void
--
--	NOTES:
--  When our QAudioOutput class changes states from, say, paused to resumed, this
--  method is invoked via signals. We do this to do some logic in regards to processing
--  data once the device is put into the idle state due to lack of data to read.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         Destructor
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      ~AndroidManager()
--
--  RETURNS:        N/A
--
--	NOTES:
--  Handles cleanup for our object when destroyed
---------------------------------------------------------------------------------------*/
AudioManager::~AudioManager() {
    delete audioOutput;
    delete circularBuffer;
    delete buffer;
    delete populateBufferWorker;
}

/*---------------------------------------------------------------------------------------
--	METHOD:         loadSong
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void loadSong(QFile* pointer to the file we want to load)
--
--  RETURNS:        void
--
--	NOTES:
--  Loads a file, begins a thread to handle it and starts that thread.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         receivedWavHeader
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void receivedWavHeader(wav_hdr wavHeader)
--
--  RETURNS:        void
--
--	NOTES:
--  When another thread receives new .wav header data for file reading it will emit
--  a signal that will invoke this slot. This slot will handle this new header data
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         playRecord
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void playRecord()
--
--  RETURNS:        void
--
--	NOTES:
--  Handles setting up pseudo .wav header data for peer-to-peer microphone communication
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         pause
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void pause()
--
--  RETURNS:        void
--
--	NOTES:
--  Handles pausing our active QAudioOutput device. Essentially encapsulates it.
---------------------------------------------------------------------------------------*/
void AudioManager::pause() {
    audioOutput->suspend();
    songState = Paused;
}

/*---------------------------------------------------------------------------------------
--	METHOD:         resume
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void resume()
--
--  RETURNS:        void
--
--	NOTES:
--  Handles resuming our active QAudioOutput device. Essentially encapsulates it.
---------------------------------------------------------------------------------------*/
void AudioManager::resume() {
    if (audioOutput->state() == QAudio::SuspendedState) {
        audioOutput->resume();
        songState = Playing;
    }
}

/*---------------------------------------------------------------------------------------
--	METHOD:         stop
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void stop()
--
--  RETURNS:        void
--
--	NOTES:
--  Handles stoping our active QAudioOutput device. Essentially encapsulates it.
---------------------------------------------------------------------------------------*/
void AudioManager::stop() {
    audioOutput->stop();
    songState = Stopped;
}

/*---------------------------------------------------------------------------------------
--	METHOD:         skip
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void skip(float amount of seconds to skip forwards or backwards)
--
--  RETURNS:        void
--
--	NOTES:
--  Handles skipping furthur or backwards within our buffer inside QAudioOutput
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         play
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      QIODevice* play()
--
--  RETURNS:        Pointer to the QIODevice being played
--
--	NOTES:
--  Handles starting to play our QIODevice/QAudioOutput. This is essentially encapsulating
--  it away.
---------------------------------------------------------------------------------------*/
QIODevice * AudioManager::play() {
    device = buffer;
    audioOutput->start(buffer);
    return buffer;
}

/*---------------------------------------------------------------------------------------
--	METHOD:         setVolume
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created [Carson]
--                  April 13th  2016: Hooked it up [Thomas]
--                  April 14th, 2016: Commented [Carson]
--
--	DESIGNERS:		Carson Roscoe / Thomas Yu
--
--	PROGRAMMER:		Carson Roscoe / Thomas Yu
--
--  INTERFACE:      void setVolume(float volume)
--
--  RETURNS:        void
--
--	NOTES:
--  Modifies the volume of the QAudioOutput being played.
---------------------------------------------------------------------------------------*/
void AudioManager::setVolume(float vol) {
    volume = vol;
    audioOutput->setVolume(volume);
}
