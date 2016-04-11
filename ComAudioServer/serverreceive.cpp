/*---------------------------------------------------------------------------------------
-- SOURCE FILE: serverreceive.cpp
--
-- PROGRAM:     ComAudioServer
--
-- FUNCTIONS:   void ShowLastErr(bool wsa);
--              int ServerReceiveSetup();
--              int ServerListen(HANDLE hFile);
--              DWORD WINAPI ServerListenThread(LPVOID lpParameter);
--              void ServerCleanup();
--              DWORD WINAPI ServerReceiveThread(LPVOID lpParameter);
--              void CALLBACK ServerCallback(DWORD Error, DWORD BytesTransferred,
--                  LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--              DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter);
--
-- DATE:        March 16, 2008
--
-- REVISIONS:   Date and Description)
--
-- DESIGNER:    Micah Willems, Carson Roscoe, Spenser Lee, Thomas Yu
--
-- PROGRAMMER:  Micah Willems, Carson Roscoe
--
-- NOTES:
--      This is a collection of functions for listening for incoming client connections
--      and receving files sent from a client.
---------------------------------------------------------------------------------------*/
///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include <QString>
#include <QBuffer>
#include <tchar.h>
#include "server.h"

////////// "Real" of the externs in Server.h ///////////////
SOCKET listenSock, acceptSock;
bool listenSockOpen, acceptSockOpen;
WSAEVENT acceptEvent;
LPSOCKET_INFORMATION SI;
char errMsg[ERRORSIZE];
int receivedSongNum = 0;

////////////////// Debug vars ///////////////////////////////
#define DEBUG_MODE
int totalbytesreceived, totalbyteswritten;


/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerReceiveSetup
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
--  INTERFACE:      int ServerReceiveSetup()
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This function sets up all the listening port info to receive file transfers.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerListen
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
--  INTERFACE:      int ServerListen(HANDLE hFile)
--
--                        HANDLE hFile: a handle to the file to write the incoming data to
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This is the qt-friendly (non-threaded) function called by the GUI to start the
--      listening for and receiving of file transfer data through threaded function calls.
---------------------------------------------------------------------------------------*/
int ServerListen()
{
    HANDLE hThread;
    DWORD ThreadId;
    LPVOID nothing;

    if ((hThread = CreateThread(NULL, 0, ServerListenThread, nothing, 0, &ThreadId)) == NULL)
    {
        sprintf_s(errMsg, "Create ServerListenThread failed with error %lu\n", GetLastError());
        qDebug() << errMsg;
        return -1;
    }
    return 0;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerListenThread
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
--  INTERFACE:      DWORD WINAPI ServerListenThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the listen thread that first initiates the listening loop, by starting
--      a thread which listens on an accepted client connection to initiate an event,
--      triggering the receive thread which loops continually on client accepts.
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerReceiveThread
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
--  INTERFACE:      DWORD WINAPI ServerReceiveThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the receiving thread for file transfer from tcp. After a client connection
--      is accepted, the event fires, the event is reset and a receive with callback is
--      made, initiating the callback loop and receive handling while maintaining listen
--      for more connection requests.
---------------------------------------------------------------------------------------*/
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
    }

    return TRUE;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerCallback
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
--  INTERFACE:      void CALLBACK ServerCallback(DWORD Error, DWORD BytesTransferred,
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
--      This is the callback thread for the WSARecv call for client file transfer. After
--      receiving the data from tcp, the number of bytes transferred is put in a slot in
--      the circular buffer followed by the data in the next slot, to distinguish how
--      much of the data is valid.
---------------------------------------------------------------------------------------*/
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
        ServerCleanup();
        GlobalFree(SI);
        return;
    }

    char slotsize[CLIENT_PACKET_SIZE];
    sprintf(slotsize, "%04lu", BytesTransferred);
    if ((circularBufferRecv->pushBack(slotsize)) == false || (circularBufferRecv->pushBack(SI->DataBuf.buf)) == false)
    {
        qDebug() << "Writing received packet to circular buffer failed";
    }

#ifdef DEBUG_MODE
    qDebug() << "\nBytes received:" << BytesTransferred;
    qDebug() << "Total bytes received:" << (totalbytesreceived += BytesTransferred);
#endif

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

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerWriteToFileThread
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
--  INTERFACE:      DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be written to
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the thread for handling processing of file transfer data received from
--      the client/sender and put into the circular buffer by the receiving callback. While the
--      receiving callback populates the circular buffer from the incoming tcp/udp stream,
--      this threaded function pulls out data items two at a time as they become
--      available, to lessen the load on the callback. For each pair of data items it
--      pulls out, the first is the number that indicates how many bytes is actual data
--      in the other. The function then checks for the delimeter indicating the last
--      packet, and aftewards writes the data to the open file.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter)
{
    DWORD byteswrittenfile = 0;
    char sizeBuf[CLIENT_PACKET_SIZE];  // first 4 bytes is size of binary data in writeBuf
    char writeBuf[CLIENT_PACKET_SIZE];
    int packetSize;
    bool lastPacket = false;
    LPCWSTR filename = (wchar_t *)calloc(25, sizeof(wchar_t));
    LPCWSTR num = (wchar_t *)calloc(4, sizeof(wchar_t));
    wsprintfW(num, "%d", receivedSongNum);
    StringCchCat(filename, 20, "Library\receivedsong");
    StringCchCat(filename, 3, num);
    StringCchCat(filename, 4, ".wav");

    CreateDirectory(TEXT("Library"), NULL);
    DeleteFile(filename);
    HANDLE hFile = CreateFile((LPCWSTR)filename, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL, NULL);

    while(!lastPacket)
    {
        if (circularBufferRecv->length > 0 && (circularBufferRecv->length % 2) == 0)
        {
            circularBufferRecv->pop(sizeBuf);
            sizeBuf[5] = '\0';
            packetSize = strtol(sizeBuf, NULL, 10);
            circularBufferRecv->pop(writeBuf);
            // check for last packet delimeter
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
                                    receivedSongNum++;
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
            if (WriteFile(hFile, writeBuf, packetSize, &byteswrittenfile, NULL) == FALSE)
            {
                qDebug() << "Couldn't write to server file\n";
                ShowLastErr(false);
                return FALSE;
            }
#ifdef DEBUG_MODE
            qDebug() << "\nBytes to write:" << packetSize;
            qDebug() << "Bytes written:" << byteswrittenfile;
            qDebug() << "Total bytes written:" << (totalbyteswritten += byteswrittenfile);
#endif
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerCleanup
--
--
--	DATE:			March 30, 2016
--
--	REVISIONS:		April 10, 2016
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      void ServerCleanup()
--
--  RETURNS:        void
--
--	NOTES:
--      This function checks every handle and socket flag on the server to see if anything
--      needs to be closed. It is used for disconnect buttons on the UI.
---------------------------------------------------------------------------------------*/
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
    if (hSendOpen)
    {
        CloseHandle(hSendFile);
        hSendOpen = false;
    }
    WSACleanup();
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ShowLastErr
--
--
--	DATE:			January 19, 2016
--
--	REVISIONS:		February 13, 2016
--
--	DESIGNERS:		Micah Willems
--
--	PROGRAMMER:		Micah Willems
--
--  INTERFACE:      void ShowLastErr()
--
--  RETURNS:        void
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
