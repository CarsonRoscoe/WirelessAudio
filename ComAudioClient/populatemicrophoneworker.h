#ifndef POPULATEMICROPHONEWORKER_H
#define POPULATEMICROPHONEWORKER_H

#include "circularbuffer.h"
#include <QBuffer>
#include <QDebug>
#include "Client.h"

extern QAudioInput * audio;
extern int micSendPacket;
extern int micPos;

class PopulateMicrophoneWorker : public QObject
{
    Q_OBJECT
public:
    PopulateMicrophoneWorker(CircularBuffer * circularBuffer, QBuffer * buffer);
    ~PopulateMicrophoneWorker() {
        alive = false;
        buffer = NULL;
    }
    void reinit(QBuffer * buffer);

public slots:
    void doWork();

private:
    CircularBuffer * circularBuffer;
    QBuffer * buffer;
    bool alive = true;
};

#endif // POPULATEMICROPHONEWORKER_H
