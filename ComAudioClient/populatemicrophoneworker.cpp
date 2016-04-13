#include "populatemicrophoneworker.h"

PopulateMicrophoneWorker::PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

void PopulateMicrophoneWorker::doWork() {
    int SendPacket = 0;
    int pos = 0;
    qDebug() << "PopulateMicrophoneWorker doWork Enter";
    while(true) {
       while (buffer->size() >= pos + SERVER_PACKET_SIZE ) {
            SendPacket++;
            char tempbuff[SERVER_PACKET_SIZE];
            int curPos = buffer->pos();
            buffer->seek(pos);
            buffer->read(tempbuff, SERVER_PACKET_SIZE);
            buffer->seek(curPos);
            pos += SERVER_PACKET_SIZE;
            //qDebug()<<"actual pos" << pos;
            circularBuffer->pushBack(tempbuff);
            qDebug() << buffer->size() << pos;
        }
        if (microphoneBuffer->size() > 1200000 ) {
            SendPacket++;
            microphoneBuffer->buffer().resize(0);
            microphoneBuffer->buffer().reserve(10000000);
            microphoneBuffer->seek(0);
            pos=0;
            microphoneBuffer->open(QIODevice::ReadWrite);
            //SendPacket=0;
            //isRecording = false;
            //return;
        }
    }
    qDebug() << "PopulateMicrophoneWorker doWork Exit";
    //buffer->close();
}
