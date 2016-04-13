#ifndef POPULATEMICROPHONEWORKER_H
#define POPULATEMICROPHONEWORKER_H

#include "circularbuffer.h"
#include <QBuffer>
#include <QDebug>
#include "Client.h"

extern QAudioInput * audio;

class PopulateMicrophoneWorker : public QObject
{
    Q_OBJECT
public:
    PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer);

public slots:
    void doWork();

private:
    CircularBuffer * circularBuffer;
    QBuffer * buffer;
};

#endif // POPULATEMICROPHONEWORKER_H
