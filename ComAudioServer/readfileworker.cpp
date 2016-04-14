#include "readfileworker.h"
#include <QDebug>

/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: ReadFileWorker.cpp
--
-- PROGRAM: ComAudioServer
--
-- FUNCTIONS:
--    ReadFileWorker(QFile * file, CircularBuffer * circularBuffer);
--    ~ReadFileWorker();
--    void doWork();
--    void load_song(QString filename);
--    void done();
--
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe & Spenser Lee
--
-- PROGRAMMER: Carson Roscoe & Spenser Lee
--
-- NOTES:
-- This final contains the functionality for reading in a file into a circular buffer.
---------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ReadFileWorker (constructor)
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe & Spenser Lee
--
-- PROGRAMMER: Carson Roscoe & Spenser Lee
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
-- FUNCTION: ~ReadFileWorker (destructor)
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe & Spenser Lee
--
-- PROGRAMMER: Carson Roscoe & Spenser Lee
--
-- INTERFACE: MainWindow(QWidget *parent)
--              parent: parent QWidget handle
--
-- RETURNS: nothing.
--
-- NOTES:
-- Constructor for the ReadFileWorker object.
---------------------------------------------------------------------------------------------------------------------*/
ReadFileWorker::~ReadFileWorker() {
    file->close();
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: doWork
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe & Spenser Lee
--
-- PROGRAMMER: Carson Roscoe & Spenser Lee
--
-- INTERFACE: doWork()
--
-- RETURNS: void
--
-- NOTES:
-- Worker function; reads file into circular buffer.
---------------------------------------------------------------------------------------------------------------------*/
void ReadFileWorker::doWork() {

    char data[SERVER_PACKET_SIZE];
    file->open(QIODevice::ReadWrite);

    while (playing) {
        while(file->read(data, SERVER_PACKET_SIZE) > 0) {
            while(!circularBuffer->pushBack(data));
        }
    }

    file->close();
    emit done();
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: load_song
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Carson Roscoe & Spenser Lee
--
-- PROGRAMMER: Carson Roscoe & Spenser Lee
--
-- INTERFACE: load_song(QString filename)
--               filename: next file to load.
--
-- RETURNS: void
--
-- NOTES:
-- Slot function to queue up next song into circular buffer.
---------------------------------------------------------------------------------------------------------------------*/
void ReadFileWorker::load_song(QString filename) {

    playing = false;
    file = new QFile(filename);

    circularBuffer->resetBuffer();

    playing = true;
    doWork();
}
