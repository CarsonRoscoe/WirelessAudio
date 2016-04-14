#include <QCoreApplication>
#include "udpthread.h"

/*---------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: UDPThread.cpp
--
-- PROGRAM: ComAudioClient
--
-- FUNCTIONS:
--    void udp_thread_request();
--    void close_socket();
--    void receive();
--
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- NOTES:
-- Process incoming UDP data.
---------------------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receive
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: void receive()
--
-- RETURNS: void
--
-- NOTES:
-- Sets up the UDP socket and loops forever on receive. If it get's data, pushes it back onto a circular buffer.
---------------------------------------------------------------------------------------------------------------------*/
void UDPThread::receive() {

    if (!clientUDP.init_socket(MULTICAST_PORT)) {
        qWarning() << "failed to init socket";
    }

    if (!clientUDP.init_multicast(MULTICAST_IP)) {
        qWarning() << "failed to set multicast settings";
    }

    while (clientUDP.receive()) {

        if (!circularBufferRecv->pushBack(clientUDP.sock_info.DataBuf.buf)) {
            continue;
        }
       emit stream_data_recv();
    }
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: udp_thread_request
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: void udp_thread_request()
--
-- RETURNS: void
--
-- NOTES:
-- Signal for launching udp thread.
---------------------------------------------------------------------------------------------------------------------*/
void UDPThread::udp_thread_request() {
    emit udp_thread_requested();
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: close_socket
--
-- DATE: APRIL 14 2016
--
-- REVISIONS: APRIL 14 2016
--
-- DESIGNER: Spenser Lee
--
-- PROGRAMMER: Spenser Lee
--
-- INTERFACE: void close_socket()
--
-- RETURNS: void
--
-- NOTES:
-- Leaves multicast and closes socket.
---------------------------------------------------------------------------------------------------------------------*/
void UDPThread::close_socket() {
    if (!clientUDP.leave_multicast()) {
        qDebug() << "failed to leave multicast";
    }

    clientUDP.close();
    this->thread()->exit();
}
