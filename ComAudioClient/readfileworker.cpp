#include "readfileworker.h"
#include "Client.h"

ReadFileWorker::ReadFileWorker(QFile * file, CircularBuffer * circularBuffer) {
    this->file = file;
    this->circularBuffer = circularBuffer;
}

void ReadFileWorker::doWork() {
    qDebug() << "ReadFileWorker doWork Enter";
    char data[SERVER_PACKET_SIZE];
    wav_hdr wavHeader;
    file->open(QIODevice::ReadOnly);
    file->read((char*)&wavHeader, sizeof(wav_hdr));
    emit gotWavHeader(wavHeader);
    //Signal for wavHeader data
    while(file->read(data, SERVER_PACKET_SIZE) > 0) {
        circularBuffer->pushBack(data);
    }
    file->close();
    qDebug() << "ReadFileWorker doWork Exit";
    emit done();
}
