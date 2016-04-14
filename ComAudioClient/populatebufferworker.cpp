#include "populatebufferworker.h"
#include "audiomanager.h"
#include "Client.h"

extern AudioManager *audioManager;
int poppacket=0;

PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

PopulateBufferWorker::~PopulateBufferWorker() {
    stayAlive = false;
}

void PopulateBufferWorker::doWork() {
    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(stayAlive) {
        while(circularBuffer->pop(buffer));
        SleepEx(25, true);
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
}
