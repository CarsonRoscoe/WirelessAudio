///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include "Client.h"

////////// "Real" of the externs in Server.h //////////////
char sendFileName[100], recvFileName[100];
char **songList = new char*[100];
SOCKET controlSock;
bool controlSockClosed = 1;

/*---------------------------------------------------------------------------------------
--	FUNCTION:   ClientSend
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
--  INTERFACE:      int ClientSend(HANDLE hFile)
--
--                      HANDLE hFile: a handle to the file to be sent
--
--  RETURNS:        Returns 0 on success, -1 on failure
--
--	NOTES:
--      This is the function that starts the thread of the control channel connection
---------------------------------------------------------------------------------------*/
int ClientSendRequest(int flag)
{
    HANDLE hThread;
    DWORD ThreadId;

    if ((hThread = CreateThread(NULL, 0, ClientControlThreadSend, (LPVOID)flag, 0, &ThreadId)) == NULL)
    {
        ShowLastErr(false);
        qDebug() << "Create Control Channel Thread failed";
        return -1;
    }
    return 0;
}

DWORD WINAPI ClientControlThreadSend(LPVOID lpParameter)
{
    char *sendbuff = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    char *recvbuff = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    char *message = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    int flag = (int)lpParameter;
    int sentb = 0, recvb = 1, totalb = 0, i = 0;
    wchar_t path[260];

    switch (flag)
    {
    case GET_UPDATE_SONG_LIST:
        memset(recvbuff, 0, sizeof(recvbuff));
        memset(message, 0, sizeof(message));
        sendbuff[0] = flag;
        sentb = send(controlSock, sendbuff, CONTROL_PACKET_SIZE, 0);
        while(recvb != SOCKET_ERROR)
        {
            recvb = recv(controlSock, recvbuff, CONTROL_PACKET_SIZE, 0);
            strcat(message, recvbuff);
            totalb += recvb;
            if (totalb >= CONTROL_PACKET_SIZE)
            {
                if (message[0] == 0)
                {
                    break;
                }
                songList[i] = new char[100];
                strcpy(songList[i], message);
                qDebug() << songList[i];
                i++;
                totalb -= CONTROL_PACKET_SIZE;
                memset(message, 0, sizeof(message));
            }
        }
        break;
    case SEND_SONG_TO_SERVER:
        sendbuff[0] = flag;
        strcat(sendbuff, sendFileName);
        sentb = send(controlSock, sendbuff, CONTROL_PACKET_SIZE, 0);
        while(totalb < CONTROL_PACKET_SIZE && recvb != SOCKET_ERROR)
        {
            recvb = recv(controlSock, recvbuff, CONTROL_PACKET_SIZE, 0);
            qDebug() << "received bytes:" << recvb;
            totalb += recvb;
        }
        if (recvbuff[0] == 0)
        {
            sendSockClosed = ClientSendSetup(address, sendSock, SERVER_DEFAULT_PORT);
            ClientSend(hSendFile);
        }
        break;
    case GET_SONG_FROM_SERVER:
        sendbuff[0] = flag;
        strcpy(recvFileName, "test.wav");
        strcat(sendbuff, recvFileName);
        mbstowcs(&path[0], recvFileName, strlen(recvFileName));
        qDebug() << recvFileName;
        qDebug() << QString::fromWCharArray(path);
        DeleteFile(path);
        hReceiveFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        ShowLastErr(false);
        hReceiveClosed = 1;
        ClientReceiveSetup(listenSock, CLIENT_DEFAULT_PORT, acceptEvent);
        ClientListen();
        sentb = send(controlSock, sendbuff, CONTROL_PACKET_SIZE, 0);
        break;
    default:
        qDebug() << "Invalid request";
        break;
    }
    free(sendbuff);
    free(recvbuff);
    free(message);
    return TRUE;
}
