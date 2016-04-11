#include "circularbuffer.h"
#include <QDebug>

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
    memcpy(front, item, elementLength);
    front = (char*)front + elementLength;
    if (front == bufferEnd) {
        front = buffer;
    }
    ++(length);
    return true;
}

bool CircularBuffer::pop(QBuffer* buf) {
    if (length == 0) {
        return false;
    }

    qint64 curPos = buf->pos();
    buf->seek(buf->size());
    buf->write((const char *)back, BUFFERSIZE);
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

bool CircularBuffer::pop(char dest[]) {
    if (length == 0) {
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
