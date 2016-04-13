#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QObject>
#include <QDebug>

#include "clientudp.h"

class UDPThread : public QObject
{
    Q_OBJECT

public:
    void udp_thread_request();

public slots:
    void receive();

signals:
    void udp_thread_requested();


private:
    ClientUDP clientUDP;

};

#endif // UDPTHREAD_H
