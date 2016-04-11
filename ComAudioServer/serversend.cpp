///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <QDebug>
#include "server.h"

////////// "Real" of the externs in Client.h ///////////////
char address[100];
SOCKET sendSock;
bool sendSockOpen;
struct sockaddr_in server;
HANDLE hSendFile;
bool hSendOpen;


int ServerSendSetup(char* addr)
{
    WSADATA WSAData;
    WORD wVersionRequested;
    struct hostent	*hp;
    strcpy(address, addr);

    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &WSAData) != 0) {
        ShowLastErr(true);
        qDebug() << "DLL not found!\n";
        return -1;
    }

    // TCP Open Socket
    if ((sendSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ShowLastErr(true);
        qDebug() << "Cannot create tcp socket\n";
        return -1;
    }

    // UDP Open Socket (if needed in future) ////////////////////
    /*if ((sendSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        qDebug() << "Cannot create udp socket\n";
        return -1;
    }*/

    // Initialize and set up the address structure
    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(CLIENT_DEFAULT_PORT);
    if ((hp = gethostbyname(address)) == NULL)
    {
        ShowLastErr(true);
        qDebug() << "Unknown server address\n";
        return -1;
    }

    // Copy the server address
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);


    // TCP Connecting to the server
    if (connect(sendSock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        ShowLastErr(true);
        qDebug() << "Can't connect to server\n";
        return -1;
    }

    sendSockOpen = true;

    // UDP Connecting to the server (if needed in future) /////////////
    // Bind local address to the socket
    /*memset((char *)&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(0);  // bind to any available port
    client.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sendSock, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("Can't bind name to socket");
        return -1;
    }*/

    qDebug() << "Setup success";
    return 0;
}

int ServerSend(HANDLE hFile)
{
    HANDLE hThread;
    DWORD ThreadId;
    hSendOpen = true;

    if ((hThread = CreateThread(NULL, 0, ServerSendThread, (LPVOID)hFile, 0, &ThreadId)) == NULL)
    {
        ShowLastErr(false);
        qDebug() << "Create Send Thread failed";
        return -1;
    }
    return 0;
}

DWORD WINAPI ServerSendThread(LPVOID lpParameter)
{
    hSendFile = (HANDLE) lpParameter;
    char *sendbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    DWORD  dwBytesRead;
    int sentBytes = 0;
    while (true) {
        if (ReadFile(hSendFile, sendbuff, SERVER_PACKET_SIZE, &dwBytesRead, NULL) == FALSE)
        {
            ShowLastErr(false);
            qDebug() << "Couldn't read file";
            ServerCleanup();
            return FALSE;
        }

        if (dwBytesRead == 0) {
            ServerCleanup();
            return TRUE;
        }
        else if (dwBytesRead > 0 && dwBytesRead < (DWORD)SERVER_PACKET_SIZE)
        {
            sendbuff[dwBytesRead] = 4;
            sendbuff[dwBytesRead + 1] = 4;
            sendbuff[dwBytesRead + 2] = 4;
        }

        // TCP Send
        sentBytes = send(sendSock, sendbuff, SERVER_PACKET_SIZE, 0);

        // UDP send (if needed in future) //////////////////////
        //sentBytes = sendto(clientparam->sock, sendbuff, clientparam->size, 0, (struct sockaddr *)&sockadd, sizeof(sockadd));
        //ShowLastErr(true);
        //sentpackets++;
    }

    return TRUE;
}
