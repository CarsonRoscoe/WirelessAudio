#include "populatebufferworker.h"

int poppacket=0;

PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

PopulateBufferWorker::~PopulateBufferWorker() {
    stayAlive = false;
    buffer->close();
}

void PopulateBufferWorker::doWork() {

    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(stayAlive) {
        if(stayAlive && circularBuffer->pop(buffer)){
            poppacket++;
        }
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
}
