#include "../ComAudioClient/circularbuffer.h"
#include <QDebug>
#include <QTimer>
int packetcounter =0;
CircularBuffer::CircularBuffer(int maxLength, int elementLength, QObject* par) : parent(par) {
    buffer = malloc(maxLength* elementLength);
    if (buffer == NULL) {
        return;
    }
    bufferEnd = (char*)buffer + maxLength * elementLength;
    this->maxLength = maxLength;
    length = 0;
    this->elementLength = elementLength;
    front = buffer;
    back = buffer;
}

CircularBuffer::~CircularBuffer() {
    free(buffer);
}

bool CircularBuffer::pushBack(void* item) {
    if (length == maxLength) {
        return false;
    }

    memcpy(front, item, elementLength);
    front = (char*)front + elementLength;
    if (front == bufferEnd) {
        front = buffer;
    }
    ++(length);
    return true;
}

bool CircularBuffer::pop(QBuffer* buf) {
    if (length < 1) {
        return false;
    }

    qint64 curPos = buf->pos();
    buf->seek(buf->size());

    try {
        buf->write((const char *)back, elementLength);
    } catch (int e) {
        qDebug() << "Errorrr";
        return false;
    }
    packetcounter++;

    buf->seek(curPos);

    back = (char*)back + elementLength;
    if (back == bufferEnd) {
        back = buffer;
    }
    --(length);

    if (buf != lastBuffer)
        lastBuffer = buf;

    return true;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   pop
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      pop(char dest[])
--
--                      char dest[]: the destination char array to copy to
--
--  RETURNS:        Returns false if the buffer is empty, otherwise true
--
--	NOTES:
--      This function removes the element from the back of the circular buffer after
--      copying it to the char array passed in
---------------------------------------------------------------------------------------*/
bool CircularBuffer::pop(char dest[]) {
    if (length < 1) {
        return false;
    }

    memcpy(dest, back, elementLength);

    back = (char*)back + elementLength;
    if (back == bufferEnd) {
        back = buffer;
    }
    --(length);

    return true;
}

void CircularBuffer::resetBuffer() {
    free(buffer);
    buffer = malloc(maxLength* elementLength);
    if (buffer == NULL) {
        return;
    }
    bufferEnd = (char*)buffer + maxLength * elementLength;
    length = 0;
    front = buffer;
    back = buffer;
}
