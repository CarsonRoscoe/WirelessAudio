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

////////// "Real" of the externs in Server.h //////////////
SOCKET listenSock, acceptSock, p2pListenSock, p2pAcceptSock;
bool listenSockOpen, acceptSockOpen, p2pListenSockOpen, p2pAcceptSockOpen;
WSAEVENT acceptEvent, p2pAcceptEvent;
HANDLE hReceiveFile;
bool hReceiveOpen;
LPSOCKET_INFORMATION SI, p2pSI;

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
    InternetAddr.sin_port = htons(P2P_DEFAULT_PORT);

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

    if ((p2pAcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
        qDebug() << "WSACreateEvent() failed with error " << WSAGetLastError();
        return -1;
    }

    qDebug() << "P2P Listening Setup Success";
    return 0;
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

    qDebug () << "Listening for P2P on port " << P2P_DEFAULT_PORT;

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
        qDebug() << "P2P Accepted a request";
        if (WSASetEvent(p2pAcceptEvent) == FALSE) {
            qDebug() << "WSASetEvent failed with error " << WSAGetLastError();
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

        //WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

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
    qDebug() << "ClientReceiveThreadP2P: Preparing to WSAWaitForMultipleEvents";
    while (TRUE) {
        Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
        qDebug() << "Event Triggered";
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
void CALLBACK ClientCallbackP2P(DWORD Error, DWORD BytesTransferred,
                                LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    DWORD RecvBytes, Flags, LastErr;
    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    p2pSI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0)
        qDebug() << "I/O operation failed with error " << Error;

    if (BytesTransferred == 0)
        qDebug() << "Closing socket " << p2pSI->Socket;

    if (Error != 0 || BytesTransferred == 0) {
        closesocket(p2pSI->Socket);
        acceptSockOpen = false;
        GlobalFree(p2pSI);
        return;
    }

    char slotsize[SERVER_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);
    if ((circularBufferRecv->pushBack(slotsize)) == false || (circularBufferRecv->pushBack(p2pSI->DataBuf.buf)) == false)
        qDebug() << "Writing received packet to circular buffer failed";

    Flags = 0;
    ZeroMemory(&(p2pSI->Overlapped), sizeof(WSAOVERLAPPED));

    p2pSI->DataBuf.len = SERVER_PACKET_SIZE;
    p2pSI->DataBuf.buf = p2pSI->Buffer;

    SleepEx(1, true);
    if (WSARecv(p2pSI->Socket, &(p2pSI->DataBuf), 1, &RecvBytes, &Flags, &(p2pSI->Overlapped), ClientCallbackP2P) == SOCKET_ERROR) {
        if ((LastErr = WSAGetLastError()) != WSA_IO_PENDING) {
            qDebug() << "WSARecv() failed with error " << LastErr;
            return;
        } else {
            SleepEx(10000, true);
        }
    }

}

//Carson, designed by Micah
DWORD WINAPI ClientWriteToFileThreadP2P(LPVOID lpParameter) {
    char sizeBuf[SERVER_PACKET_SIZE];
    char writeBuf[SERVER_PACKET_SIZE];
    int packetSize;
    bool lastPacket = false;

    qDebug() << "Enter ClientWriteToFileP2P";
    while(!lastPacket) {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0) {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            for (int a = 0, b = 1, c = 2, d = 3, e = 4;
                 a < packetSize - 5; a++, b++, c++, d++, e++)
            {
                if (writeBuf[a] == 'm') {
                    if (writeBuf[b] == 'i') {
                        if (writeBuf[c] == 'l') {
                            if (writeBuf[d] == 'e') {
                                if (writeBuf[e] == 'd') {
                                    packetSize = a - 1;
#ifdef DEBUG_MODE
                                    qDebug() << "Last packet";
#endif
                                    break;
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
                        a++;
                        b++;
                        c++;
                        d++;
                        e++;
                    }
                }
            }

            int cur = listeningBuffer->pos();
            int newpos = listeningBuffer->size() > 0 ? listeningBuffer->size() : 0;
            listeningBuffer->seek(newpos);
            listeningBuffer->write(writeBuf, packetSize);
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
