#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <QDebug>

#include "server.h"

class ServerUDP
{
public:
    ServerUDP(){}
    ~ServerUDP(){}

    bool init_socket(short port);
    bool init_multicast(const char *name);
    bool broadcast_message(char *message, LPDWORD lp_bytes_sent);

private:
    SOCKET_INFORMATION  sock_info;
    SOCKADDR_IN         local_addr;
    WSADATA             wsa_data;
    DWORD               flags = 0;

    struct ip_mreq  multicast_addr;
    SOCKADDR_IN     dest_addr;
};

#endif // SERVERUDP_H
