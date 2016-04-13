#include "populatemicrophoneworker.h"

PopulateMicrophoneWorker::PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateMicrophoneWorker::doWork() {
    char tempbuff[SERVER_PACKET_SIZE];
    int SendPacket = 0;
    int pos = 0;
    qDebug() << "PopulateMicrophoneWorker doWork Enter";
    while(true) {
       while (buffer->size() >= pos + SERVER_PACKET_SIZE ) {
            ++SendPacket;
            int curPos = buffer->pos();
            buffer->seek(pos);
            buffer->read(tempbuff, SERVER_PACKET_SIZE);
            buffer->seek(curPos);
            pos += SERVER_PACKET_SIZE;
            circularBuffer->pushBack(tempbuff);
        }
        if (microphoneBuffer->size() > 1200000 ) {
            ++SendPacket;
            microphoneBuffer->buffer().resize(0);
            microphoneBuffer->buffer().reserve(10000000);
            microphoneBuffer->seek(0);
            pos=0;
            microphoneBuffer->open(QIODevice::ReadWrite);
        }
    }
    qDebug() << "PopulateMicrophoneWorker doWork Exit";
}
