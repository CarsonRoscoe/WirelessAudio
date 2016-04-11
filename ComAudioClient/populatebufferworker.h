#ifndef POPULATEBUFFERWORKER_H
#define POPULATEBUFFERWORKER_H

#include "circularbuffer.h"
#include <QBuffer>
#include <QDebug>

class PopulateBufferWorker : public QObject {
    Q_OBJECT
public:
    PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer);

public slots:
    void doWork();

private:
    CircularBuffer * circularBuffer;
    QBuffer * buffer;
};

#endif // POPULATEBUFFERWORKER_H
