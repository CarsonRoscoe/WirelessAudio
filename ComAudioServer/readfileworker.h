#ifndef READFILETHREAD_H
#define READFILETHREAD_H

#include <QThread>
#include <QFile>
#include "server.h"
#include "../ComAudioClient/circularbuffer.h"

class ReadFileWorker : public QObject {
    Q_OBJECT
public:
    ReadFileWorker(QFile * file, CircularBuffer * circularBuffer);
    ~ReadFileWorker();

public slots:
    void doWork();
    void load_song(QString filename);

signals:
    void done();

private:
    QFile * file;
    CircularBuffer * circularBuffer;
    bool playing = true;
};

#endif // READFILETHREAD_H
