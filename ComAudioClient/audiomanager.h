#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QAudioFormat>
#include <QAudioOutput>
#include <QFile>
#include <QDebug>
#include <QMetaType>
#include "wavheader.h"
#include "songstate.h"
#include "circularbuffer.h"
#include "readfileworker.h"
#include "populatebufferworker.h"

//Carson
class AudioManager : public QObject {
Q_OBJECT
public:
    AudioManager(QObject * parent);
    ~AudioManager();

    void pause();
    void resume();
    void skip(float seconds);
    QIODevice * play();
    void setVolume(float volume);
    void loadSong(QFile * file);
    void stop();
    void playRecord();
    void Init(QBuffer * buf);

public slots:
    void receivedWavHeader(wav_hdr wavHeader);

private:
    QAudioFormat format;
    QAudioOutput *audio;
    QObject *parent;
    QFile *file;
    QIODevice *device;
    float volume = 10;
    SongState songState;
    CircularBuffer *circularBuffer;
    QBuffer *buffer;
    ReadFileWorker *readFileWorker;
    PopulateBufferWorker *populateBufferWorker;
    QThread readWorkerThread;
    QThread populateBufferThread;
    int bytesPerSecond;
};

#endif // AUDIOMANAGER_H
