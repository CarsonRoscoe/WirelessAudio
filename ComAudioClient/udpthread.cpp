#include "udpthread.h"

void UDPThread::receive() {

    qDebug() << "thread created";

    if (!clientUDP.init_socket(7000)) {
        qDebug() << "failed to init socket";
    }

    if (!clientUDP.init_multicast("234.5.6.7")) {
        qDebug() << "failed to set multicast settings";
    }

//    while (clientUDP.receive()) {

    while (1) {

        // check if header packet (with file type info)
        // push back on to circular buffer clientUDP.sock_info.Databuf.buf
        // emit signal indicating song header (process in mainwindow to recreate QAudioOutput)

        // if not header packet
        // append to circular buff client.UDP.sock_info.Databuf.buf
        // emit signal indicating more song data acquired (process in mainwindow)

        if (!clientUDP.receive()) {
            qDebug() << "receive failed";
        } else {
            qDebug() << clientUDP.sock_info.DataBuf.buf;
        }

        Sleep(1000);
    }

    qDebug() << "thread exiting";
}

void UDPThread::udp_thread_request() {
    emit udp_thread_requested();
}
