#include "populatemicrophoneworker.h"

int micSendPacket = 0;
int micPos = 0;

PopulateMicrophoneWorker::PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateMicrophoneWorker::reinit(QBuffer * buffer) {
    this->buffer = buffer;
    micSendPacket = 0;
    micPos = 0;
}

void PopulateMicrophoneWorker::doWork() {
    qDebug() << "PopulateMicrophoneWorker doWork Enter";
    while(alive) {
       while (alive && buffer->size() >= micPos + SERVER_PACKET_SIZE ) {
            if (!alive || buffer == NULL)
                return;
            micSendPacket++;
            char tempbuff[SERVER_PACKET_SIZE];
            int curPos = buffer->pos();
            buffer->seek(micPos);
            buffer->read(tempbuff, SERVER_PACKET_SIZE);
            buffer->seek(curPos);
            micPos += SERVER_PACKET_SIZE;
            circularBuffer->pushBack(tempbuff);
        }
    }
    qDebug() << "PopulateMicrophoneWorker doWork Exit";
}
