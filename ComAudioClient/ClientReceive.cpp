/*---------------------------------------------------------------------------------------
-- SOURCE FILE: clientreceive.cpp
--
-- PROGRAM:     ComAudioClient
--
-- FUNCTIONS:   int ClientReceiveSetup();
--              int ClientListen(HANDLE hFile);
--              DWORD WINAPI ClientListenThread(LPVOID lpParameter);
--              DWORD WINAPI ClientReceiveThread(LPVOID lpParameter);
--              void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
--                  LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--              DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter);
--
-- DATE:        April 3, 2016
--
-- REVISIONS:
--
-- DESIGNER:    Micah Willems, Carson Roscoe, Spenser Lee, Thomas Yu
--
-- PROGRAMMER:  Micah Willems, Carson Roscoe
--
-- NOTES:
--      This is a collection of functions for connecting to and receving files from
--      a server
---------------------------------------------------------------------------------------*/
///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <QDebug>
#include <QString>
#include <QBuffer>
#include "Client.h"
#include <QFile>
#include <QTextStream>

////////// "Real" of the externs in Server.h //////////////
SOCKET listenSock, acceptSock, p2pListenSock, p2pAcceptSock;
bool listenSockClosed = 1, acceptSockClosed = 1, p2pListenSockClosed = 1, p2pAcceptSockClosed = 1;
WSAEVENT acceptEvent, p2pAcceptEvent;
HANDLE hReceiveFile;
bool hReceiveClosed = 1;
LPSOCKET_INFORMATION SI, p2pSI;
bool start;

//////////////////// Debug vars ///////////////////////////
#define DEBUG_MODE
int totalbytesreceived, totalbyteswritten;

//////////////////// File Globals /////////////////////////
bool listenAccept;

//Carson, designed by Micah
int ClientReceiveSetupP2P() {
    int ret;
    WSADATA wsaData;
    SOCKADDR_IN InternetAddr;

    start = false;

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

    int option = 1;
    setsockopt(p2pListenSock, SOL_SOCKET,SO_REUSEADDR, (char*)&option, sizeof(option));

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(P2P_DEFAULT_PORT);

    if (bind(p2pListenSock, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR) {
        qDebug() << "bind failed with error " << WSAGetLastError();
        return -1;
    }

    // TCP listen on socket
    if (listen(p2pListenSock, 5)) {
        qDebug() << "listen() failed with error " << WSAGetLastError();
        return -1;
    }

    p2pListenSockClosed = 0;

    if ((p2pAcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
        qDebug() << "WSACreateEvent() failed with error " << WSAGetLastError();
        return -1;
    }
}



/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientReceiveSetup
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      int ClientReceiveSetup()
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This function sets up all the listening port info to receive file transfers.
---------------------------------------------------------------------------------------*/
int ClientReceiveSetup(SOCKET &sock, int port, WSAEVENT &event)
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
    if ((sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        sprintf_s(errMsg, "Failed to get a socket %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }


    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (bind(sock, (PSOCKADDR)&InternetAddr,
        sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        sprintf_s(errMsg, "bind() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }
    // TCP listen on socket (no corresponding UDP call)
    if (listen(sock, 1))
    {
        sprintf_s(errMsg, "listen() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }

    if ((event = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        sprintf_s(errMsg, "WSACreateEvent() failed with error %d\n", WSAGetLastError());
        qDebug() << errMsg;
        return -1;
    }

    return 0;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientListen
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      int ClientListen(HANDLE hFile)
--
--                        HANDLE hFile: a handle to the file to write the incoming data to
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This is the qt-friendly (non-threaded) function called by the GUI to start the
--      listening for and receiving of file transfer data through threaded function calls.
---------------------------------------------------------------------------------------*/
int ClientListen()
{
    HANDLE hThread;
    DWORD ThreadId;
    hReceiveClosed = 0;

    if ((hThread = CreateThread(NULL, 0, ClientListenThread, 0, 0, &ThreadId)) == NULL)
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

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientListenThread
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      DWORD WINAPI ClientListenThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the listen thread that first initiates the listening loop, by starting
--      a thread which listens on an accepted server connection to initiate an event,
--      triggering the receive thread.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ClientListenThread(LPVOID lpParameter)
{
    HANDLE hThread;
    DWORD ThreadId;
    listenAccept = 1;

    if ((hThread = CreateThread(NULL, 0, ClientReceiveThread, lpParameter, 0, &ThreadId)) == NULL)
    {
        sprintf_s(errMsg, "Create ClientReceiveThread failed with error %lu\n", GetLastError());
        qDebug() << errMsg;
        return FALSE;
    }

    while (listenAccept)
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
    listenAccept = true;

    if ((hThread = CreateThread(NULL, 0, ClientReceiveThreadP2P, lpParameter, 0, &ThreadId)) == NULL) {
        qDebug() << "Create ServerReceiveThread failed with error " << GetLastError();
        return FALSE;
    }

    while (listenAccept) {
        if ((p2pAcceptSock = accept(p2pListenSock, NULL, NULL)) == INVALID_SOCKET) {
            qDebug() << "ClientListenThread Error:" << WSAGetLastError();
            return FALSE;
        }
        if (WSASetEvent(p2pAcceptEvent) == FALSE) {
            qDebug() << "P2P WSASetEvent failed with error" << WSAGetLastError();
            return FALSE;
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientReceiveThread
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      DWORD WINAPI ClientReceiveThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the receiving thread for file transfer from tcp. After a server connection
--      is accepted, the event fires, and a receive with a callback thread is
--      made, initiating the callback loop and receive handling.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ClientReceiveThread(LPVOID lpParameter)
{
    WSAEVENT EventArray[1];
    HANDLE hThread;
    DWORD Index, RecvBytes, Flags, LastErr, ThreadId;
    LPSOCKET_INFORMATION SocketInfo;
    totalbytesreceived = 0;
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

        if (acceptSock == -1)
            return TRUE;

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
        acceptSockClosed = true;


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
    p2pAcceptSockClosed = 1;

    Flags = 0;
    // TCP WSA receive

    if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, &(SocketInfo->Overlapped), ClientCallbackP2P) == SOCKET_ERROR) {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug() << "WSARecv() failed with error " << LastErr;
            return FALSE;
        }
    }

    SleepEx(10000, true);

    return TRUE;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientCallback
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
--                      LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
--                      DWORD Error: error message resulting from WSARecv
--                      DWORD BytesTransferred: the number of bytes received from tcp
--                      LPWSAOVERLAPPED Overlapped: the overlapped structure, to be used
--                          in the next WSARecv call
--                      DWORD InFlags: unused
--
--  RETURNS:        void
--
--	NOTES:
--      This is the callback thread for the WSARecv call for server file transfer. After
--      receiving the data from tcp, the number of bytes transferred is put in a slot in
--      the circular buffer followed by the data in the next slot, to distinguish how
--      much of the data is valid.
---------------------------------------------------------------------------------------*/
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
        listenAccept = 0;
        closesocket(SI->Socket);
        closesocket(listenSock);
        closesocket(acceptSock);
        listenSockClosed = 1;
        acceptSockClosed = 1;
        GlobalFree(SI);
        return;
    }

    char slotsize[SERVER_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);

    while ((circularBufferRecv->pushBack(slotsize)) == false){}
    while ((circularBufferRecv->pushBack(SI->DataBuf.buf)) == false){}

#ifdef DEBUG_MODE
    qDebug() << "\nBytes received:" << BytesTransferred;
    qDebug() << "Total bytes received:" << (totalbytesreceived += BytesTransferred);
#endif

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

void CleanupRecvP2P() {
    GlobalFree(p2pSI);
    closesocket(p2pListenSock);
    closesocket(p2pAcceptSock);
    p2pListenSockClosed = 1;
    p2pAcceptSockClosed = 1;
    WSACleanup();
    p2pAcceptEvent = NULL;
    listeningBuffer->buffer().clear();
    listeningBuffer->buffer().resize(0);
    listeningBuffer->close();
    listeningBuffer->setBuffer(new QByteArray());
    listeningBuffer->seek(listeningBuffer->size());
    listeningBuffer->open(QIODevice::ReadWrite);
    isRecording = false;
    listenAccept = false;
}

//Carson, designed by Micah
void CALLBACK ClientCallbackP2P(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    DWORD RecvBytes, Flags, LastErr;
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION p2pSI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0)
        qDebug() << "I/O operation failed with error " << Error;

    if (BytesTransferred == 0)
        qDebug() << "Closing socket " << p2pSI->Socket;

    if (Error != 0 || BytesTransferred == 0) {
        ClientCleanup();
        GlobalFree(p2pSI);
        return;
    }
      if (circularBufferRecv->pushBack(p2pSI->DataBuf.buf) == false)
          qDebug() << "error pushing back CR " << p2pSI->Socket;
        Flags = 0;

     if (!start) {
        start = true;
        startP2PAudio = true;
        qDebug() << "Start reading audio";
     }

    if(packetcounter==146){
        CleanupRecvP2P();
        ClientReceiveSetupP2P();
        ClientListenP2P();
        packetcounter = 0;
        return;
    }

    if (WSARecv(p2pSI->Socket, &(p2pSI->DataBuf), 1, &p2pSI->BytesRECV, &Flags, &(p2pSI->Overlapped), ClientCallbackP2P) == SOCKET_ERROR) {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug() << "WSARecv() failed with error " << LastErr;
            return;
        } else {
            SleepEx(25000, true);
        }
    }
}

//Carson, designed by Micah
DWORD WINAPI ClientWriteToFileThreadP2P(LPVOID lpParameter) {
    char sizeBuf[SERVER_PACKET_SIZE];
    char writeBuf[SERVER_PACKET_SIZE];
    int packetSize;
    bool lastPacket = false;
    hReceiveFile = (HANDLE) lpParameter;

    while(!lastPacket) {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0) {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            int cur = listeningBuffer->pos();
            listeningBuffer->write(writeBuf);
            listeningBuffer->seek(cur);
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientWriteToFileThread
--
--
--	DATE:			April 7, 2016
--
--	REVISIONS:
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the thread for handling processing of file transfer data received from
--      the server/sender and put into the circular buffer by the receiving callback. While the
--      receiving callback populates the circular buffer from the incoming tcp/udp stream,
--      this threaded function pulls out data items two at a time as they become
--      available, to lessen the load on the callback. For each pair of data items it
--      pulls out, the first is the number that indicates how many bytes is actual data
--      in the other. The function then checks for the delimeter indicating the last
--      packet, and aftewards writes the data to the open file.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter) {
    DWORD byteswrittenfile = 0;
    char sizeBuf[SERVER_PACKET_SIZE];
    char writeBuf[SERVER_PACKET_SIZE];
    int packetSize;
    bool lastPacket = false;
    totalbyteswritten = 0;

    while(!lastPacket)
    {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0)
        {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            for (int a = 0, b = 1, c = 2, d = 3, e = 4;
                 a < packetSize - 5; a++, b++, c++, d++, e++)
            {
                if (writeBuf[a] == 'd') {
                    if (writeBuf[b] == 'e') {
                        if (writeBuf[c] == 'l') {
                            if (writeBuf[d] == 'i') {
                                if (writeBuf[e] == 'm') {
                                    lastPacket = true;
                                    packetSize = a;
#ifdef DEBUG_MODE
                                    qDebug() << "Last packet";
#endif
                                    break;
                                }
                            } else {
                                a += 4;
                                b += 4;
                                c += 4;
                                d += 4;
                                e += 4;
                            }
                        } else {
                            a += 3;
                            b += 3;
                            c += 3;
                            d += 3;
                            e += 3;
                        }
                    } else {
                        a += 2;
                        b += 2;
                        c += 2;
                        d += 2;
                        e += 2;
                    }
                } else {
                    a ++;
                    b ++;
                    c ++;
                    d ++;
                    e ++;
                }
            }
            if (WriteFile(hReceiveFile, writeBuf, packetSize, &byteswrittenfile, NULL) == FALSE)
            {
                ShowLastErr(false);
                qDebug() << "Couldn't write to server file\n";
                return FALSE;
            }
#ifdef DEBUG_MODE
            qDebug() << "\nBytes to write:" << packetSize;
            qDebug() << "Bytes written:" << byteswrittenfile;
            qDebug() << "Total bytes written:" << (totalbyteswritten += byteswrittenfile);
#endif
        }
    }
    CloseHandle(hReceiveFile);
    hReceiveClosed = 1;
    return TRUE;
}
