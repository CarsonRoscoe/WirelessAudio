#include "readfileworker.h"
#include "Client.h"
/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: ReadFileWorker.cpp
--
-- PROGRAM: ComAudioServer
--
-- FUNCTIONS:
--    ReadFileWorker(QFile * file, CircularBuffer * circularBuffer);
--    ~ReadFileWorker();
--    void doWork();
--
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- This file contains the functionality for reading in a file into a circular buffer.
---------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ReadFileWorker (constructor)
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: ReadFileWorker(QFile * file, CircularBuffer * circularBuffer)
--              file:               file to read into circular buffer
--              circularBuffer:     buffer to load file into
--
-- RETURNS: nothing.
--
-- NOTES:
-- Constructor for the MainWindow object.
---------------------------------------------------------------------------------------------------------------------*/
ReadFileWorker::ReadFileWorker(QFile * file, CircularBuffer * circularBuffer) {
    this->file = file;
    this->circularBuffer = circularBuffer;
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: doWork
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: doWork()
--
-- RETURNS: void
--
-- NOTES:
-- Worker function; reads file into circular buffer.
---------------------------------------------------------------------------------------------------------------------*/
void ReadFileWorker::doWork() {
    qDebug() << "ReadFileWorker doWork Enter";
    char data[SERVER_PACKET_SIZE];
    wav_hdr wavHeader;
    file->open(QIODevice::ReadOnly);
    file->read((char*)&wavHeader, sizeof(wav_hdr));
    emit gotWavHeader(wavHeader);
    //Signal for wavHeader data
    while(file->read(data, SERVER_PACKET_SIZE) > 0) {
        while (!circularBuffer->pushBack(data));
    }
    file->close();
    qDebug() << "ReadFileWorker doWork Exit";
    emit done();
}
