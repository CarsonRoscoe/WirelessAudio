#include "clientudp.h"

bool ClientUDP::init_socket(short port) {

    int opt = 1;

    // load winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        qDebug() << "WSAStartup() failed: " << WSAGetLastError();
        return false;
    }

    // create udp socket
    if ((sock_info.Socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        qDebug() << "WSASocket() failed: " << WSAGetLastError();
        return false;
    }

    // set resuse addr
    if (setsockopt(sock_info.Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
    {
        qDebug() << "WSASocket() failed: " << WSAGetLastError();
        return false;
    }

    local_addr.sin_family       = AF_INET;
    local_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    local_addr.sin_port         = htons(port);

    if (bind(sock_info.Socket, (LPSOCKADDR) &local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        qDebug() << "bind() failed: " << WSAGetLastError();
        return false;
    }

    return true;
}

bool ClientUDP::init_multicast(const char *name) {


    multicast_addr.imr_multiaddr.s_addr = inet_addr(name);
    multicast_addr.imr_interface.s_addr = INADDR_ANY;


    // Join the multicast group
    if(setsockopt(sock_info.Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multicast_addr, sizeof(multicast_addr)) == SOCKET_ERROR) {
        qDebug() << "setsockopt() on multicast address failed: " << WSAGetLastError();
        return false;
    }

    return true;
}

bool ClientUDP::receive() {

    memset(sock_info.Buffer, '\0', 10);  // TODO: use buffer size define

    sock_info.DataBuf.len = 10;
    sock_info.DataBuf.buf = sock_info.Buffer;

    ZeroMemory(&sock_info.Overlapped, sizeof(WSAOVERLAPPED));
    sock_info.Overlapped.hEvent = WSACreateEvent();

    if (WSARecvFrom(
                sock_info.Socket,
                &sock_info.DataBuf,
                1, NULL, &flags,
                (SOCKADDR*)&src_addr,
                (LPINT)&src_len,
                &sock_info.Overlapped,
                NULL) == SOCKET_ERROR) {

        if (WSAGetLastError() == WSA_IO_PENDING) {

            if (WSAWaitForMultipleEvents(1, &sock_info.Overlapped.hEvent, false, INFINITE, false) == WAIT_TIMEOUT) {
                qDebug() << "WSARecvFrom() timeout";
                return false;
            }

            if (!WSAGetOverlappedResult(sock_info.Socket, &(sock_info.Overlapped), &sock_info.BytesRECV, false, &flags)) {
                qDebug() << "UDP WSAGetOverlappedResult() failed: " << WSAGetLastError();
                return false;
            }
        } else {
            qDebug() << "WSARecvFrom() failed: " << WSAGetLastError();
        }
    }

    return true;
}





















