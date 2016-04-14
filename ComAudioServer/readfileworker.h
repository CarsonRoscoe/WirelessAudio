#ifndef READFILETHREAD_H
#define READFILETHREAD_H

#include <QThread>
#include <QFile>
#include "server.h"
#include "circularbuffer.h"

class ReadFileWorker : public QObject {
    Q_OBJECT
public:
    ReadFileWorker(QFile * file, CircularBuffer * circularBuffer);

public slots:
    void doWork();

signals:
    void done();

private:
    QFile * file;
    CircularBuffer * circularBuffer;
};

#endif // READFILETHREAD_H
