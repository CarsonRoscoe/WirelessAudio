#include "populatebufferworker.h"

PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateBufferWorker::doWork() {
    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(true) {
        circularBuffer->pop(buffer);
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
    buffer->close();
}
