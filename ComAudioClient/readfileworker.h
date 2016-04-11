#ifndef READFILETHREAD_H
#define READFILETHREAD_H

#include <QThread>
#include <QFile>
#include "circularbuffer.h"
#include "wavheader.h"

class ReadFileWorker : public QObject {
    Q_OBJECT
public:
    ReadFileWorker(QFile * file, CircularBuffer * circularBuffer);

public slots:
    void doWork();

signals:
    void done();
    void gotWavHeader(wav_hdr header);

private:
    QFile * file;
    CircularBuffer * circularBuffer;
};

#endif // READFILETHREAD_H
