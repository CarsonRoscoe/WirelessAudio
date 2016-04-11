///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <QDebug>
#include <QString>
#include <QBuffer>
#include "Client.h"

////////// "Real" of the externs in Server.h //////////////
SOCKET listenSock, acceptSock, p2pListenSock, p2pAcceptSock;
bool listenSockOpen, acceptSockOpen, p2pListenSockOpen, p2pAcceptSockOpen;
WSAEVENT acceptEvent, p2pAcceptEvent;
HANDLE hReceiveFile;
bool hReceiveOpen;
LPSOCKET_INFORMATION SI;

//Carson, designed by Micah
int ClientReceiveSetupP2P() {
    int ret;
    WSADATA wsaData;
    SOCKADDR_IN InternetAddr;

    if ((ret = WSAStartup(0x0202, &wsaData)) != 0) {
        qDebug() << "WSAStartup failed with error " << ret;
        WSACleanup();
        return -1;
    }

    // TCP create WSA socket
    if ((p2pListenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        qDebug() << "Failed tog et a socket " << WSAGetLastError();
        return -1;
    }

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(CLIENT_DEFAULT_PORT);

    if (bind(p2pListenSock, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR) {
        qDebug() << "bind) failed with error " << WSAGetLastError();
        return -1;
    }

    // TCP listen on socket
    if (listen(p2pListenSock, 5)) {
        qDebug() << "listen() failed with error " << WSAGetLastError();
        return -1;
    }

    p2pListenSockOpen = true;

    if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
        qDebug() << "WSACreateEvent() failed with error " << WSAGetLastError();
        return -1;
    }

    qDebug() << "P2P Listening Setup Success";
    return 0;
}

//Micah
int ClientReceiveSetup()
{
    int ret;
    WSADATA wsaData;
    SOCKADDR_IN InternetAddr;

    if ((ret = WSAStartup(0x0202, &wsaData)) != 0)
    {
        sprintf_s(errMsg, ERRORSIZE, "WSAStartup failed with error %d\n", ret);
        qDebug() << errMsg;
        WSACleanup();
        return -1;
    }

    // TCP create WSA socket
    if ((listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        sprintf_s(errMsg, "Failed to get a socket %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }

    // UDP create WSA socket (if needed in future) ////////////////////////
    /*if ((listenSock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        sprintf_s(errMsg, "Failed to get a socket %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }*/


    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(CLIENT_DEFAULT_PORT);

    if (bind(listenSock, (PSOCKADDR)&InternetAddr,
        sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        sprintf_s(errMsg, "bind() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }
    // TCP listen on socket (no corresponding UDP call)
    if (listen(listenSock, 5))
    {
        sprintf_s(errMsg, "listen() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }
    listenSockOpen = true;

    if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        sprintf_s(errMsg, "WSACreateEvent() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }

    qDebug() << "Success!";
    return 0;
}

//Micah
int ClientListen(HANDLE hFile)
{
    HANDLE hThread;
    DWORD ThreadId;
    hReceiveOpen = true;

    if ((hThread = CreateThread(NULL, 0, ClientListenThread, (LPVOID) hFile, 0, &ThreadId)) == NULL)
    {
        sprintf_s(errMsg, "Create ServerListenThread failed with error %lu\n", GetLastError());
        qDebug() << errMsg;
        return -1;
    }
    return 0;
}

//Carson, designed by Micah
int ClientListenP2P() {
    HANDLE hThread;
    DWORD ThreadId;

    if ((hThread = CreateThread(NULL, 0, ClientListenThreadP2P, 0, 0, &ThreadId)) == NULL) {
        qDebug() << "Create ClientListenP2P failed with error " << GetLastError();
        return -1;
    }

    return 0;
}

//Micah
DWORD WINAPI ClientListenThread(LPVOID lpParameter)
{
    HANDLE hThread;
    DWORD ThreadId;

    if ((hThread = CreateThread(NULL, 0, ClientReceiveThread, lpParameter, 0, &ThreadId)) == NULL)
    {
        sprintf_s(errMsg, "Create ServerReceiveThread failed with error %lu\n", GetLastError());
        qDebug() << errMsg;
        return FALSE;
    }

    while (TRUE)
    {
        acceptSock = accept(listenSock, NULL, NULL);

        if (WSASetEvent(acceptEvent) == FALSE)
        {
            sprintf_s(errMsg, "WSASetEvent failed with error %d\n", WSAGetLastError());
            qDebug() << errMsg;
            return FALSE;
        }
    }
    return TRUE;
}

//Carson, designed by Micah
DWORD WINAPI ClientListenThreadP2P(LPVOID lpParameter) {
    HANDLE hThread;
    DWORD ThreadId;

    if ((hThread = CreateThread(NULL, 0, ClientReceiveThreadP2P, lpParameter, 0, &ThreadId)) == NULL) {
        qDebug() << "Create ServerReceiveThread failed with error " << GetLastError();
        return FALSE;
    }

    while (TRUE) {
        p2pAcceptSock = accept(p2pListenSock, NULL, NULL);

        if (WSASetEvent(p2pAcceptEvent) == FALSE) {
            qDebug() << "WSASetEvent failed with error " << WSAGetLastError();
            return FALSE;
        }
    }

    return TRUE;
}

//Micah
DWORD WINAPI ClientReceiveThread(LPVOID lpParameter)
{
    WSAEVENT EventArray[1];
    HANDLE hThread;
    DWORD Index, RecvBytes, Flags, LastErr, ThreadId;
    LPSOCKET_INFORMATION SocketInfo;

    // Save the accept event in the event array.

    EventArray[0] = acceptEvent;

    while (TRUE)
    {
        // Wait for accept() to signal an event and also process WorkerRoutine() returns.

        while (TRUE)
        {
            Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

            if (Index == WSA_WAIT_FAILED)
            {
                sprintf_s(errMsg, "WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
                qDebug() << errMsg;
                return FALSE;
            }

            if (Index != WAIT_IO_COMPLETION)
            {
                // An accept() call event is ready - break the wait loop
                break;
            }
        }

        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

        // Create a socket information structure to associate with the accepted socket.

        if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            sprintf_s(errMsg, "GlobalAlloc() failed with error %d\n", GetLastError());
            qDebug() << errMsg;
            return FALSE;
        }

        // Fill in the details of our accepted socket.

        SocketInfo->Socket = acceptSock;
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->BytesSEND = 0;
        SocketInfo->BytesRECV = 0;
        SocketInfo->DataBuf.len = SERVER_PACKET_SIZE;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;

        sprintf_s(errMsg, "Socket %d connected\n", acceptSock);
        acceptSockOpen = true;
        qDebug() << errMsg;


        Flags = 0;
        // TCP WSA receive
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            &(SocketInfo->Overlapped), ClientCallback) == SOCKET_ERROR)
        {
            if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING)
            {
                sprintf_s(errMsg, "WSARecv() failed with error %d\n", LastErr);
                qDebug() << errMsg;
                return FALSE;
            }
        }

        if ((hThread = CreateThread(NULL, 0, ClientWriteToFileThread, lpParameter, 0, &ThreadId)) == NULL)
        {
            sprintf_s(errMsg, "Create ServerWriteToFileThread failed with error %lu\n", GetLastError());
            qDebug() << errMsg;
            return FALSE;
        }

        // UDP WSA receive (if needed in future) //////////////////////////////////
        /*if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            (SOCKADDR *)&ClientAddr, &clientaddrsize, &(SocketInfo->Overlapped), ClientCallback) == SOCKET_ERROR)
        {
            ShowLastErr(true);
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                sprintf_s(errMsg, "WSARecv() failed with error %d\n", WSAGetLastError());
                qDebug() << errMsg;
                return FALSE;
            }
        }*/
    }

    return TRUE;
}

//Carson, designer by Micah
DWORD WINAPI ClientReceiveThreadP2P(LPVOID lpParameter) {
    WSAEVENT EventArray[1];
    HANDLE hThread;
    DWORD Index, RecvBytes, Flags, LastErr, ThreadId;
    LPSOCKET_INFORMATION SocketInfo;

    // Save the accept event in the event array.
    EventArray[0] = p2pAcceptEvent;

    // Wait for accept() to signal an event and also process WorkerRoutine() returns.
    while (TRUE) {
        Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

        if (Index == WSA_WAIT_FAILED) {
            qDebug() << "WSAWaitForMultipleEvents failed with error " << WSAGetLastError();
            return FALSE;
        }

        //An accept() call event is ready - break the wait loop
        if (Index != WAIT_IO_COMPLETION) {
            break;
        }
    }

    // Create a socket information structure to associate with the accepted socket.
    if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL) {
        qDebug() << "GlobalAlloc() failed with error " << GetLastError();
        return FALSE;
    }

    // Fill in the details of our accepted socket.
    SocketInfo->Socket = p2pAcceptSock;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->BytesSEND = 0;
    SocketInfo->BytesRECV = 0;
    SocketInfo->DataBuf.len = SERVER_PACKET_SIZE;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    sprintf_s(errMsg, "Socket %d connected\n", p2pAcceptSock);
    p2pAcceptSockOpen = true;
    qDebug() << errMsg;

    Flags = 0;
    // TCP WSA receive
    if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, &(SocketInfo->Overlapped), ClientCallbackP2P) == SOCKET_ERROR) {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug() << "WSARecv() failed with error " << LastErr;
            return FALSE;
        }
    }

    if ((hThread = CreateThread(NULL, 0, ClientWriteToFileThreadP2P, lpParameter, 0, &ThreadId)) == NULL) {
        qDebug() << "Create ServerWriteToFileThread failed with error " << GetLastError();
        return FALSE;
    }

    return TRUE;
}

//Micah
void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD RecvBytes, Flags, LastErr;
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    SI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0)
    {
        sprintf_s(errMsg, "I/O operation failed with error %d\n", Error);
        qDebug() << errMsg;
    }

    if (BytesTransferred == 0)
    {
        sprintf_s(errMsg, "Closing socket %d\n", SI->Socket);
        qDebug() << errMsg;
    }

    if (Error != 0 || BytesTransferred == 0)
    {
        closesocket(SI->Socket);
        acceptSockOpen = false;
        GlobalFree(SI);
        return;
    }

    char slotsize[SERVER_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);
    if ((circularBufferRecv->pushBack(slotsize)) == false || (circularBufferRecv->pushBack(SI->DataBuf.buf)) == false)
    {
        qDebug() << "Writing received packet to circular buffer failed";
    }

    Flags = 0;
    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    SI->DataBuf.len = SERVER_PACKET_SIZE;
    SI->DataBuf.buf = SI->Buffer;

    if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
        &(SI->Overlapped), ClientCallback) == SOCKET_ERROR)
    {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING)
        {
            sprintf_s(errMsg, "WSARecv() failed with error %d\n", LastErr);
            qDebug() << errMsg;
            return;
        }
    }
}

//Carson, designed by Micah
void CALLBACK ClientCallbackP2P(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    DWORD RecvBytes, Flags, LastErr;
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    SI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0)
        qDebug() << "I/O operation failed with error " << Error;

    if (BytesTransferred == 0)
        qDebug() << "Closing socket " << SI->Socket;

    if (Error != 0 || BytesTransferred == 0) {
        closesocket(SI->Socket);
        acceptSockOpen = false;
        GlobalFree(SI);
        return;
    }

    char slotsize[SERVER_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);
    if ((circularBufferRecv->pushBack(slotsize)) == false || (circularBufferRecv->pushBack(SI->DataBuf.buf)) == false)
        qDebug() << "Writing received packet to circular buffer failed";

    Flags = 0;
    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    SI->DataBuf.len = SERVER_PACKET_SIZE;
    SI->DataBuf.buf = SI->Buffer;

    if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), ClientCallbackP2P) == SOCKET_ERROR) {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug() << "WSARecv() failed with error " << LastErr;
            return;
        }
    }
}

//Carson, designed by Micah
DWORD WINAPI ClientWriteToFileThreadP2P(LPVOID lpParameter) {
    DWORD byteswrittenfile = 0;
    char sizeBuf[SERVER_PACKET_SIZE];
    char writeBuf[SERVER_PACKET_SIZE];
    char delim[6] = {(int)'d', (int)'e', (int)'l', (int)'i', (int)'m', '\0'}, *ptrEnd, *ptrBegin = writeBuf;
    int packetSize;
    bool lastPacket = false;
    hReceiveFile = (HANDLE) lpParameter;

    while(!lastPacket) {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0) {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            if ((ptrEnd = strstr(writeBuf, delim))) {
                lastPacket = true;
                packetSize = ptrEnd - ptrBegin;
            }
            int cur = listeningBuffer->pos();
            listeningBuffer->write(writeBuf);
            listeningBuffer->seek(cur);
        }
    }
    return TRUE;
}


//Micah
DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter) {
    DWORD byteswrittenfile = 0;
    char sizeBuf[SERVER_PACKET_SIZE];
    char writeBuf[SERVER_PACKET_SIZE];
    char delim[6] = {(int)'d', (int)'e', (int)'l', (int)'i', (int)'m', '\0'}, *ptrEnd, *ptrBegin = writeBuf;
    int packetSize;
    bool lastPacket = false;
    hReceiveFile = (HANDLE) lpParameter;

    while(!lastPacket)
    {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0)
        {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            if ((ptrEnd = strstr(writeBuf, delim)))
            {
                lastPacket = true;
                packetSize = ptrEnd - ptrBegin;
            }
            if (WriteFile(hReceiveFile, writeBuf, packetSize, &byteswrittenfile, NULL) == FALSE)
            {
                qDebug() << "Couldn't write to server file\n";
                ShowLastErr(false);
                return FALSE;
            }
        }
    }
    return TRUE;
}