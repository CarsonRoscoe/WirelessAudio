#include "readfileworker.h"
#include <QDebug>

ReadFileWorker::ReadFileWorker(QFile * file, CircularBuffer * circularBuffer) {
    this->file = file;
    this->circularBuffer = circularBuffer;
}

void ReadFileWorker::doWork() {
    qDebug() << "ReadFileWorker doWork Enter";
    char data[SERVER_PACKET_SIZE];
//    wav_hdr wavHeader;
    file->open(QIODevice::ReadWrite);
//    file->read((char*)&wavHeader, sizeof(wav_hdr));
//    emit gotWavHeader(wavHeader);
    //Signal for wavHeader data

    qDebug() << circularBuffer->length;
    while(file->read(data, SERVER_PACKET_SIZE) > 0) {
        while(!circularBuffer->pushBack(data));
    }
    qDebug() << circularBuffer->length;
    file->close();
    qDebug() << "ReadFileWorker doWork Exit";
    emit done();
}
