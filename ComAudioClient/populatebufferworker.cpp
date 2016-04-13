#include "populatebufferworker.h"

int poppacket=0;

PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateBufferWorker::doWork() {

    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(true) {
        if(circularBuffer->pop(buffer))
            poppacket++;
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
    buffer->close();
}
