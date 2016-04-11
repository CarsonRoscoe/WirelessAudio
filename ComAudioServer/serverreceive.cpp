///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include <QString>
#include <QBuffer>
#include "server.h"

////////// "Real" of the externs in Server.h ///////////////
SOCKET listenSock, acceptSock;
bool listenSockOpen, acceptSockOpen;
WSAEVENT acceptEvent;
HANDLE hReceiveFile;
bool hReceiveOpen;
LPSOCKET_INFORMATION SI;
char errMsg[ERRORSIZE];


int ServerReceiveSetup()
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
    InternetAddr.sin_port = htons(SERVER_DEFAULT_PORT);

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

int ServerListen(HANDLE hFile)
{
    HANDLE hThread;
    DWORD ThreadId;
    hReceiveOpen = true;

    if ((hThread = CreateThread(NULL, 0, ServerListenThread, (LPVOID) hFile, 0, &ThreadId)) == NULL)
    {
        sprintf_s(errMsg, "Create ServerListenThread failed with error %lu\n", GetLastError());
        qDebug() << errMsg;
        return -1;
    }
    return 0;
}

DWORD WINAPI ServerListenThread(LPVOID lpParameter)
{
    HANDLE hThread;
    DWORD ThreadId;

    if ((hThread = CreateThread(NULL, 0, ServerReceiveThread, lpParameter, 0, &ThreadId)) == NULL)
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

DWORD WINAPI ServerReceiveThread(LPVOID lpParameter)
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
        SocketInfo->DataBuf.len = CLIENT_PACKET_SIZE;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;

        sprintf_s(errMsg, "Socket %d connected\n", acceptSock);
        acceptSockOpen = true;
        qDebug() << errMsg;

        Flags = 0;
        // TCP WSA receive
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            &(SocketInfo->Overlapped), ServerCallback) == SOCKET_ERROR)
        {
            if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING)
            {
                sprintf_s(errMsg, "WSARecv() failed with error %d\n", LastErr);
                qDebug() << errMsg;
                return FALSE;
            }
        }

        if ((hThread = CreateThread(NULL, 0, ServerWriteToFileThread, lpParameter, 0, &ThreadId)) == NULL)
        {
            sprintf_s(errMsg, "Create ServerWriteToFileThread failed with error %lu\n", GetLastError());
            qDebug() << errMsg;
            return FALSE;
        }
        // UDP WSA receive (if needed in future) //////////////////////////////////
        /*if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            (SOCKADDR *)&ClientAddr, &clientaddrsize, &(SocketInfo->Overlapped), ServerCallback) == SOCKET_ERROR)
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

void CALLBACK ServerCallback(DWORD Error, DWORD BytesTransferred,
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

    char slotsize[CLIENT_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);
    if ((circularBufferRecv->pushBack(slotsize)) == false || (circularBufferRecv->pushBack(SI->DataBuf.buf)) == false)
    {
        qDebug() << "Writing received packet to circular buffer failed";
    }

    Flags = 0;
    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    SI->DataBuf.len = CLIENT_PACKET_SIZE;
    SI->DataBuf.buf = SI->Buffer;

    if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
        &(SI->Overlapped), ServerCallback) == SOCKET_ERROR)
    {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING)
        {
            sprintf_s(errMsg, "WSARecv() failed with error %d\n", LastErr);
            qDebug() << errMsg;
            return;
        }
    }
}

DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter)
{
    DWORD byteswrittenfile = 0;
    char sizeBuf[circularBufferRecv->elementLength];
    char writeBuf[circularBufferRecv->elementLength];
    char delim[4] = {4, 4, 4, '\0'}, *ptrEnd, *ptrBegin = writeBuf;
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

void ServerCleanup()
{
    if (sendSockOpen)
    {
        closesocket(sendSock);
        sendSockOpen = false;
    }
    if (acceptSockOpen)
    {
        closesocket(acceptSock);
        acceptSockOpen = false;
    }
    if (listenSockOpen)
    {
        closesocket(listenSock);
        listenSockOpen = false;
    }
    if (hReceiveOpen)
    {
        CloseHandle(hReceiveFile);
        hReceiveOpen = false;
    }
    if (hSendOpen)
    {
        CloseHandle(hSendFile);
        hSendOpen = false;
    }
    WSACleanup();
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:		void ShowLastErr()

--
--	DATE:			January 19, 2016
--
--	REVISIONS:		February 13, 2016
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--	NOTES:
--      This is a universal-purpose error reporting function for functions that can be
--		error checked using GetLastError. It uses FormatMessage to get an actual
--		human-readable, understandable string from the error ID and outputs it to the
--		Debug output console in the IDE.
---------------------------------------------------------------------------------------*/
void ShowLastErr(bool wsa)
{
    DWORD dlasterr;
    LPCTSTR errMsg = NULL;
    char errnum[100];
    if (wsa == true)
    {
        dlasterr = WSAGetLastError();
    }
    else
    {
        dlasterr = GetLastError();
    }
    sprintf_s(errnum, "Error number: %d\n", dlasterr);
    qDebug() << errnum;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, dlasterr, 0, (LPWSTR)&errMsg, 0, NULL);
    qDebug() << QString::fromWCharArray(errMsg);
}
