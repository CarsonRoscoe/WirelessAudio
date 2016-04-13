#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <QDebug>

#include "Client.h"

class ClientUDP
{
public:
    ClientUDP() {}
    ~ClientUDP() {}

    bool init_socket(short port);
    bool init_multicast(const char *name);
    bool receive();

    SOCKET_INFORMATION  sock_info;

private:

    SOCKADDR_IN         local_addr;
    WSADATA             wsa_data;
    DWORD               flags = 0;
    DWORD               src_len = sizeof(local_addr);

    struct ip_mreq  multicast_addr;
    SOCKADDR_IN     src_addr;

};

#endif // CLIENTUDP_H
