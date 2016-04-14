/*---------------------------------------------------------------------------------------------------------------------
-- CLASS FILE: circularbuffer.cpp
--
-- PROGRAM: ComAudioClient
--
-- METHODS:
--    Constructor(int maxLength, int elementLength, QObject* par) : parent(par)
--    ~Destructor()
--    bool pushBack(void* item)
--    bool pop(QBuffer* buf)
--    bool pop(char dest[])
--    void resetBuffer()
--
-- DATE: April 1st 2016
--
-- REVISIONS: April 1st 2016:   Created
--            April 13th 2016:  Integrated
--            APril 14th, 2016: Commented
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- Circular buffer created to hold data in a reliable way while reading/writing threads did their own work, allowing
-- for less work do be done on the sending/receiving threads themselves and allowing faster performance overall.
---------------------------------------------------------------------------------------------------------------------*/

#include "../ComAudioClient/circularbuffer.h"
#include <QDebug>
#include <QTimer>

int packetcounter =0;

/*---------------------------------------------------------------------------------------
--	METHOD:         Constructor
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      CircularBuffer(int maximum amount of nodes
--                                 int maximum amount of bytes per node
--                                 QObject* parent window)
--
--  RETURNS:        N/A
--
--	NOTES:
--  Constructor for our circular buffer class.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	METHOD:         Destructor
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      ~CircularBuffer()
--
--  RETURNS:        N/A
--
--	NOTES:
--  Cleanup code for the circular buffer. Essentially just free's it all.
---------------------------------------------------------------------------------------*/
CircularBuffer::~CircularBuffer() {
    free(buffer);
}

/*---------------------------------------------------------------------------------------
--	METHOD:         pushBack
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      bool pushBack(void* item)
--
--  RETURNS:        true or false over whether the push back occured or not
--
--	NOTES:
--  Writes the data passed in into a node at the front of the circular buffer. If this
--  was successful, the method returns true. If the method WOULD overwrite an existing
--  node, the method refuses to do so and simply returns false.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION:       pop
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      pop(QBuffer* the destination char array to copy to)
--
--  RETURNS:        Returns false if the buffer is empty, otherwise true
--
--	NOTES:
--  This function removes the element from the back of the circular buffer after
--  copying it to the char array passed in. It appends the new data to the end
--  of the QBuffer.
---------------------------------------------------------------------------------------*/
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

    //qDebug() << "Buffer Size:" << buf->size() << "Buffer Pos:" << curPos;


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
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      pop(char dest[] the destination char array to copy to
--
--  RETURNS:        Returns false if the buffer is empty, otherwise true
--
--	NOTES:
--  This function removes the element from the back of the circular buffer after
--  copying it to the char array passed in
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

/*---------------------------------------------------------------------------------------
--	FUNCTION:       resetBuffer
--
--	DATE:			April 1st, 2016
--
--	REVISIONS:      April 1st   2016: Created
--                  April 13th  2016: Integrated
--                  April 14th, 2016: Commented
--
--	DESIGNERS:		Carson Roscoe
--
--	PROGRAMMER:		Carson Roscoe
--
--  INTERFACE:      void resetBuffer()
--
--  RETURNS:        void
--
--	NOTES:
--  Resets the data within the circular buffer. Essentially it is both a desctructor
--  and a constructor to be used on the fly when you want to reset the buffer.
---------------------------------------------------------------------------------------*/
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
