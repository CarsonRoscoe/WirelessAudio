#include "populatebufferworker.h"

PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateBufferWorker::doWork() {
    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(true) {
        //while (buffer->size() - buffer->pos() < BUFFERSIZE * 10) {
            circularBuffer->pop(buffer);
        //}
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
    buffer->close();
}
