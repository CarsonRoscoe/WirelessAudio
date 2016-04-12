#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <Windows.h>
#include "../ComAudioClient/circularbuffer.h"

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT	7001
#define CLIENT_DEFAULT_PORT	7002
#define CONTROL_PORT        7004
#define FILENAMESIZE		100
#define ERRORSIZE			512
#define CLIENT_PACKET_SIZE  8192
#define SERVER_PACKET_SIZE  8192
#define MAX_CLIENTS         5

////////////////// Control Channel Flags ///////////////////
#define GET_UPDATE_SONG_LIST    1
#define SEND_SONG_TO_SERVER     2
#define GET_SONG_FROM_SERVER    3

///////////// Global Structure Definitions ////////////////
struct ClientParams {
    bool tcp;
    char filename[FILENAMESIZE];
    int numpackets;
    long size;
    SOCKET sock;
    struct sockaddr_in server;
    WSAEVENT accept;
};

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[CLIENT_PACKET_SIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

///////////////////// Global Variables ////////////////////
// Receiving
extern SOCKET listenSock, acceptSock;
extern bool listenSockClosed, acceptSockClosed;
extern struct sockaddr_in server;
extern WSAEVENT acceptEvent, defaultEvent;
extern LPSOCKET_INFORMATION SI;
extern char errMsg[ERRORSIZE];
extern CircularBuffer* circularBufferRecv;
extern char recvFileName[100];
// Sending
extern char address[100];
extern SOCKET sendSock[MAX_CLIENTS];
extern bool sendSockClosed;
extern HANDLE hSendFile;
extern bool hSendClosed;
extern struct sockaddr_in server;
extern char sendFileName[100];
// Control Channel
extern SOCKET controlSock, Clients[MAX_CLIENTS];
extern bool controlSockOpen, clientClosed[MAX_CLIENTS];
extern struct sockaddr_in client;
extern char **songList;
extern int numClients, numSongs;

///////////////////// Global Prototypes ///////////////////
// Receiving
void ShowLastErr(bool wsa);
int ServerReceiveSetup(SOCKET &sock, int port, bool noEvent, WSAEVENT &event = defaultEvent);
int ServerListen();
DWORD WINAPI ServerListenThread(LPVOID lpParameter);
void ServerCleanup();
DWORD WINAPI ServerReceiveThread(LPVOID lpParameter);
void CALLBACK ServerCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter);
// Sending
int ServerSendSetup(char* addr, int clientID);
int ServerSend(HANDLE hFile);
DWORD WINAPI ServerSendThread(LPVOID lpParameter);
// Control Channel
void GetSongList();
DWORD WINAPI ServerCreateControlChannels(LPVOID lpParameter);
DWORD WINAPI ServerListenControlChannel(LPVOID lpParameter);

#endif
