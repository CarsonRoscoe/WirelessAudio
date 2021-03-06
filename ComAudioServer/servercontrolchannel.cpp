///////////////////// Includes ////////////////////////////
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include <QString>
#include <QFile>
#include <io.h>
#include "server.h"

////////// "Real" of the externs in Server.h ///////////////
SOCKET controlSock, Clients[MAX_CLIENTS];
bool controlSockOpen, clientClosed[MAX_CLIENTS] = {1};
struct sockaddr_in client;
char **ipClients = new char*[MAX_CLIENTS];
char **songList = new char*[100];
int numSongs;

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
    int	client_len, clientSlot;
    HANDLE hThread;
    DWORD ThreadId;

    while(true)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientClosed[i] == 1)
            {
                clientSlot = i;
                break;
            }
        }
        client_len = sizeof(client);
        if ((Clients[clientSlot] = accept(controlSock, (struct sockaddr *)&client, &client_len)) == -1)
        {
            qDebug() << "Can't accept client\n";
            return FALSE;
        }

        ipClients[clientSlot] = new char[100];
        strcpy(ipClients[clientSlot], inet_ntoa(client.sin_addr));
        qDebug() << "IP of new client:" << ipClients[clientSlot];
        qDebug() << "Control socket" << Clients[clientSlot] << "connected";
        clientClosed[clientSlot] = 0;

        if ((hThread = CreateThread(NULL, 0, ServerListenControlChannel, (LPVOID)clientSlot, 0, &ThreadId)) == NULL)
        {
            sprintf_s(errMsg, "Create ServerListenControlChannel failed with error %lu\n", GetLastError());
            qDebug() << errMsg;
            return -1;
        }
    }
}

DWORD WINAPI ServerListenControlChannel(LPVOID lpParameter)
{
    int sentb = 0, recvb = 1, totalb = 0;
    char *sendbuff = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    char *recvbuff = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    char *message = (char *)calloc(CONTROL_PACKET_SIZE + 1, sizeof(char));
    int clientID = (int)lpParameter;
    wchar_t path[260];
    QFile *file;

    while(true)
    {
        memset(sendbuff, 0, sizeof(sendbuff));
        memset(recvbuff, 0, sizeof(sendbuff));
        memset(message, 0, sizeof(message));
        while(recvb != 0 && recvb != SOCKET_ERROR)
        {
            recvb = recv(Clients[clientID], recvbuff, CONTROL_PACKET_SIZE, 0);
            strcat(message, recvbuff);
            totalb += recvb;
            if (recvb == -1 || recvb == 0) {
                qDebug() << "Socket" << Clients[clientID] << "closed";
                closesocket(Clients[clientID]);
                free(sendbuff);
                free(recvbuff);
                free(message);
                return TRUE;
            }
            if (totalb >= CONTROL_PACKET_SIZE)
            {
                switch (message[0])
                {
                case GET_UPDATE_SONG_LIST:
                    GetSongList();
                    memset(sendbuff, 0, sizeof(sendbuff));
                    for (int i = 0; i < numSongs; i++) {
                        strcpy(sendbuff, songList[i]);
                        sentb = send(Clients[clientID], songList[i], CONTROL_PACKET_SIZE, 0);
                    }
                    sendbuff[0] = 0;
                    sentb = send(Clients[clientID], sendbuff, CONTROL_PACKET_SIZE, 0);
                    break;
                case SEND_SONG_TO_SERVER:
                    memmove(message, message+1, strlen(message) - 1);
                    message[strlen(message) - 1] = 0;
                    strcpy(recvFileName, "\.\\Library\\");
                    strcat(recvFileName, message);
                    listenSockClosed = ServerReceiveSetup(listenSock, SERVER_DEFAULT_PORT, false, acceptEvent);
                    ServerListen();
                    sendbuff[0] = listenSockClosed;
                    memset(sendbuff + 1, 88, 20);
                    sentb = send(Clients[clientID], sendbuff, CONTROL_PACKET_SIZE, 0);
                    ShowLastErr(true);
                    break;
                case GET_SONG_FROM_SERVER:
                    memmove(message, message+1, strlen(message) - 1);
                    message[strlen(message) - 1] = 0;
                    strcpy(sendFileName, "./Library/");
                    strcat(sendFileName, message);
                    file = new QFile(QString::fromLatin1(sendFileName));
                    file->open(QIODevice::ReadOnly);
                    hFile = (HANDLE) _get_osfhandle(file->handle());
                    ShowLastErr(false);
                    hClosed = 0;
                    sendSockClosed = ServerSendSetup(ipClients[clientID], clientID);
                    ServerSend(clientID);
                    break;
                default:
                    break;
                }
                memset(message, 0, sizeof(message));
                totalb -= CONTROL_PACKET_SIZE;
            }
        }
    }
}
