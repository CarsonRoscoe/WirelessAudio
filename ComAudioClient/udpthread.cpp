#include <QCoreApplication>
#include "udpthread.h"

void UDPThread::receive() {

    qDebug() << "thread created";

    if (!clientUDP.init_socket(4985)) {
        qDebug() << "failed to init socket";
    }

    if (!clientUDP.init_multicast("234.5.6.7")) {
        qDebug() << "failed to set multicast settings";
    }

    while (clientUDP.receive()) {

        qDebug() << "got song data";

        if (!circularBufferRecv->pushBack(clientUDP.sock_info.DataBuf.buf)) {
            continue;
        }
       emit stream_data_recv();
    }
    qDebug() << "thread exiting";
}

void UDPThread::udp_thread_request() {
    emit udp_thread_requested();
}
