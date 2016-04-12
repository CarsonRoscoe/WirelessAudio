///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include <QString>
#include "server.h"

////////// "Real" of the externs in Server.h ///////////////
SOCKET controlSock, Clients[MAX_CLIENTS];
bool controlSockOpen, clientOpen[MAX_CLIENTS] = {false};
struct sockaddr_in client;
char **songList = new char*[100];
int numClients = 0, numSongs;

void GetSongList()
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;
    numSongs = 0;

    hFind = FindFirstFile(TEXT("\.\\Library\\*"), &ffd);
    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            songList[numSongs] = new char[100];
            wcstombs(songList[numSongs], ffd.cFileName, 100);
            numSongs++;
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
        if ((Clients[numClients] = accept(controlSock, (struct sockaddr *)&client, &client_len)) == -1)
        {
            qDebug() << "Can't accept client\n";
            return FALSE;
        }

        qDebug() << "Control socket" << Clients[numClients] << "connected";
        clientOpen[numClients] = true;

        if ((hThread = CreateThread(NULL, 0, ServerListenControlChannel, (LPVOID)numClients, 0, &ThreadId)) == NULL)
        {
            sprintf_s(errMsg, "Create ServerListenControlChannel failed with error %lu\n", GetLastError());
            qDebug() << errMsg;
            return -1;
        }
        numClients++;
    }
}

DWORD WINAPI ServerListenControlChannel(LPVOID lpParameter)
{
    int sentb = 0, recvb = 1, totalb;
    char *sendbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    char *recvbuff = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    char *message = (char *)calloc(SERVER_PACKET_SIZE + 1, sizeof(char));
    int clientID = (int)lpParameter;

    while(true)
    {
        while(recvb != 0 && recvb != SOCKET_ERROR)
        {
            recvb = recv(Clients[clientID], recvbuff, SERVER_PACKET_SIZE, 0);
            strcat(message, recvbuff);
            totalb += recvb;
            if (totalb >= SERVER_PACKET_SIZE)
            {
                switch (message[0])
                {
                case GET_UPDATE_SONG_LIST:
                    GetSongList();
                    for (int i = 0; i < numSongs; i++) {
                        sentb = send(Clients[clientID], songList[i], SERVER_PACKET_SIZE, 0);
                    }
                    break;
                case SEND_SONG_TO_SERVER:
                    memmove(message, message+1, strlen(message) - 1);
                    message[strlen(message) - 1] = 0;
                    strcpy(recvFileName, "\.\\Library\\");
                    strcat(recvFileName, message);
                    listenSockClosed = ServerReceiveSetup(listenSock, SERVER_DEFAULT_PORT, false, acceptEvent);
                    ServerListen();
                    sendbuff[0] = listenSockClosed;
                    sentb = send(Clients[clientID], sendbuff, SERVER_PACKET_SIZE, 0);
                    break;
                case GET_SONG_FROM_SERVER:
                    break;
                default:
                    break;
                }
                memset(message, 0, sizeof(message));
                totalb -= SERVER_PACKET_SIZE;
            }
        }
    }
}
