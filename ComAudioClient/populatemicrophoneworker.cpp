/*---------------------------------------------------------------------------------------
-- SOURCE FILE: populatebuffer.cpp
--
-- PROGRAM:     ComAudioClient
--
-- FUNCTIONS:  PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer)
--             void reinit(QBuffer * buffer)
--             void doWork()
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

#include "populatemicrophoneworker.h"

int micSendPacket = 0;
int micPos = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:PopulateMicrophoneWorker::PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer)
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

PopulateMicrophoneWorker::PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer) {
    this->circularBuffer = circularBuffer;
    this->buffer = buffer;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:void PopulateMicrophoneWorker::reinit(QBuffer * buffer)
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE:void PopulateMicrophoneWorker::reinit(QBuffer * buffer)
--           QBuffer * buffer: The QBuffer that pushes back data into the circular buffer.
-- RETURNS: void
--
-- NOTES:
-- This reassigns a qbuffer that push backs into the circular buffer.
----------------------------------------------------------------------------------------------------------------------*/
void PopulateMicrophoneWorker::reinit(QBuffer * buffer) {
    this->buffer = buffer;
    micSendPacket = 0;
    micPos = 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:PopulateMicrophoneWorker::doWork()
--
-- DATE: April 13, 2016
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE:PopulateMicrophoneWorker::doWork()
--
-- RETURNS: void
--
-- NOTES:
-- This starts pushing back data into the circular buffer from the QBuffer.
----------------------------------------------------------------------------------------------------------------------*/
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
