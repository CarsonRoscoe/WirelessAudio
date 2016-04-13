#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <QDebug>

#include "server.h"

/* Socket struct for Windows */
//typedef struct _SOCKET_INFORMATION
//{
//    WSAOVERLAPPED  Overlapped;
//    SOCKET         Socket;
//    CHAR           Buffer[DATA_BUFSIZE];
//    WSABUF         DataBuf;
//    DWORD          BytesSEND;
//    DWORD          BytesRECV;
//} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

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
