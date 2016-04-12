///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include "server.h"

////////// "Real" of the externs in Server.h ///////////////
SOCKET controlSock, Clients[MAX_CLIENTS];
bool controlSockOpen, clientOpen[MAX_CLIENTS] = {false};
struct sockaddr_in client;
char *songList[100];
int currClient = 0;

void GetSongList()
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;

    hFind = FindFirstFile(TEXT("\.\\Library\\*"), &ffd);
    do
    {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         qDebug() << ffd.cFileName;
      }
   }
   while (FindNextFile(hFind, &ffd) != 0);
}

DWORD WINAPI ServerCreateControlChannels(LPVOID lpParameter)
{
    int	client_len;
    HANDLE hThread;
    DWORD ThreadId;

    while(true)
    {
        client_len = sizeof(client);
        if ((Clients[currClient] = accept(controlSock, (struct sockaddr *)&client, &client_len)) == -1)
        {
            qDebug() << "Can't accept client\n";
            return FALSE;
        }
        clientOpen[currClient] = true;
        if ((hThread = CreateThread(NULL, 0, ServerListenControlChannel, (LPVOID)currClient, 0, &ThreadId)) == NULL)
        {
            sprintf_s(errMsg, "Create ServerListenThread failed with error %lu\n", GetLastError());
            qDebug() << errMsg;
            return -1;
        }
        currClient++;
    }
}

DWORD WINAPI ServerListenControlChannel(LPVOID lpParameter)
{
    int sentBytes = 0, err = 1;
    char *sendbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    char *recvbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    char *message = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    int clientID = (int)lpParameter;

    while(true)
    {
        while(err != 0 && err != SOCKET_ERROR)
        {
            err = recv(controlSock, recvbuff, SERVER_PACKET_SIZE, 0);
            strcat(message, recvbuff);
        }
        switch (message[0])
        {
        case GET_UPDATE_SONG_LIST:
            break;
        case SEND_SONG_TO_SERVER:
            break;
        case GET_SONG_FROM_SERVER:
            break;
        default:
            qDebug() << "Error, invalid flag";
            break;
        }
    }
}
