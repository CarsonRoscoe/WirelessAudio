/*---------------------------------------------------------------------------------------
-- SOURCE FILE: populatebuffer.cpp
--
-- PROGRAM:     ComAudioClient
--
-- FUNCTIONS:   PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer)
--              ~PopulateBufferWorker()
--              void PopulateBufferWorker::doWork()
--
-- DATE:        April 2, 2016
--
-- REVISIONS:
--
-- DESIGNER:    Carson Roscoe
--
-- PROGRAMMER:  Carson Roscoe
--
-- NOTES:
--      This is a "worker" that populates a QBuffer from a circular buffer
---------------------------------------------------------------------------------------*/

#include "populatebufferworker.h"
#include "audiomanager.h"
#include "Client.h"

extern AudioManager *audioManager;
int poppacket=0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE:PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer)
--          CircularBuffer * circularBuffer: the circular buffer
--          QBuffer * buffer: The Qbuffer
-- RETURNS: N/A : Constructor
--
-- NOTES:
-- This is the constructor to assign which buffer to populate from which QBuffer.
----------------------------------------------------------------------------------------------------------------------*/
PopulateBufferWorker::PopulateBufferWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:~PopulateBufferWorker()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE:~PopulateBufferWorker()

-- RETURNS: N/A : destructor
--
-- NOTES:
-- This is the destructor that closes the buffer and sets a flag.
----------------------------------------------------------------------------------------------------------------------*/
PopulateBufferWorker::~PopulateBufferWorker() {
    stayAlive = false;
    buffer->close();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:void PopulateBufferWorker::doWork()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE:void PopulateBufferWorker::doWork()

-- RETURNS: void
--
-- NOTES:
-- This starts popping from the circularbuffer into the buffer.
----------------------------------------------------------------------------------------------------------------------*/
void PopulateBufferWorker::doWork() {
    buffer->open(QIODevice::ReadWrite);
    qDebug() << "PopulateBufferWorker doWork Enter";
    while(stayAlive) {
        while(circularBuffer->pop(buffer));
        SleepEx(25, true);
    }
    qDebug() << "PopulateBufferWorker doWork Exit";
}
