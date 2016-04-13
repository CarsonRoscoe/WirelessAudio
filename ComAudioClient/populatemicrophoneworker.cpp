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
            qDebug()<<buffer->bytesAvailable();
            // if(buffer->bytesAvailable()>=SERVER_PACKET_SIZE){
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
           // }
          //  else
        }
       /*
        if (SendPacket == 10) {
            SendPacket=0;
            isRecording = false;
            pos = 0;
            audio->stop();
            delete buffer;
            buffer = new QBuffer();
            buffer->open(QIODevice::ReadWrite);
            //buffer->buffer().reserve(2000000);
            audio->start(buffer);
        }
        */
    }
    qDebug() << "PopulateMicrophoneWorker doWork Exit";
}
