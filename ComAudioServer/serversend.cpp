/*---------------------------------------------------------------------------------------
-- SOURCE FILE: serversend.cpp
--
-- PROGRAM:     ComAudioServer
--
-- FUNCTIONS:   int ServerSendSetup(char* addr)
--              int ServerSend(HANDLE hFile)
--              DWORD WINAPI ServerSendThread(LPVOID lpParameter)
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
--      This is a collection of functions for connecting and sending files to a client
---------------------------------------------------------------------------------------*/
///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <QDebug>
#include "server.h"

////////// "Real" of the externs in Client.h ///////////////
char address[100];
SOCKET sendSock[MAX_CLIENTS];
bool sendSockClosed = 1;
struct sockaddr_in server;
HANDLE hSendFile;
bool hSendClosed = 1;
char sendFileName[100];

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerSendSetup
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
--  INTERFACE:      int ServerSendSetup(char* addr)
--
--                      char* addr: the ip address to connect to
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This function sets up the socket for sending a file to the client
---------------------------------------------------------------------------------------*/
int ServerSendSetup(char* addr, int clientID)
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
    if ((sendSock[clientID] = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ShowLastErr(true);
        qDebug() << "Cannot create tcp socket\n";
        return -1;
    }

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
    if (connect(sendSock[clientID], (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        ShowLastErr(true);
        qDebug() << "Can't connect to server\n";
        return -1;
    }

    qDebug() << "Setup success";
    return 0;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerSend
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
--  INTERFACE:      int ServerSend(HANDLE hFile)
--
--                      HANDLE hFile: a handle to the file to be sent
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This is the function that starts the thread to send a file to the client
---------------------------------------------------------------------------------------*/
int ServerSend(int clientID)
{
    HANDLE hThread;
    DWORD ThreadId;
    hSendClosed = 0;

    if ((hThread = CreateThread(NULL, 0, ServerSendThread, (LPVOID)clientID, 0, &ThreadId)) == NULL)
    {
        ShowLastErr(false);
        qDebug() << "Create Send Thread failed";
        return -1;
    }
    return 0;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ServerSendThread
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
--  INTERFACE:      DWORD WINAPI ServerSendThread(LPVOID lpParameter)
--
--                      LPVOID lpParameter: the handle to the file to be sent
--
--  RETURNS:        Returns TRUE on success, FALSE on failure
--
--	NOTES:
--      This is the threaded function that sends a file to the client. When the last
--      portion of the data is read, a delimeter is appended to indicate that it's the
--      end on the client's side.
---------------------------------------------------------------------------------------*/
DWORD WINAPI ServerSendThread(LPVOID lpParameter)
{
    int clientID = (int) lpParameter;
    char *sendbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    DWORD  dwBytesRead;
    int sentBytes = 0;
    while (true) {
        if (ReadFile(hReceivedFile, sendbuff, SERVER_PACKET_SIZE, &dwBytesRead, NULL) == FALSE)
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
        // if less than SERVER_PACKET_SIZE was read, then end-of-file must have been encountered
        else if (dwBytesRead > 0 && dwBytesRead < (DWORD)SERVER_PACKET_SIZE)
        {
            sendbuff[dwBytesRead] = 'd';
            sendbuff[dwBytesRead + 1] = 'e';
            sendbuff[dwBytesRead + 2] = 'l';
            sendbuff[dwBytesRead + 2] = 'i';
            sendbuff[dwBytesRead + 2] = 'm';
        }

        // TCP Send
        sentBytes = send(sendSock, sendbuff, SERVER_PACKET_SIZE, 0);
    }
    free(sendbuff);
    return TRUE;
}
