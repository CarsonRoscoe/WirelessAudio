#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <Windows.h>
#include "circularbuffer.h"

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT	7001
#define CLIENT_DEFAULT_PORT	7002
#define FILENAMESIZE		100
#define ERRORSIZE			512
#define CLIENT_PACKET_SIZE  8192
#define SERVER_PACKET_SIZE  8192

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
extern struct sockaddr_in server;
extern WSAEVENT acceptEvent;
extern HANDLE hReceiveFile;
extern bool hReceiveOpen;
extern LPSOCKET_INFORMATION SI;
extern char errMsg[ERRORSIZE];
extern CircularBuffer* circularBufferRecv;
// Sending
extern char address[100];
extern SOCKET sendSock;
extern bool sendSockOpen;
extern HANDLE hSendFile;
extern bool hSendOpen;
extern struct sockaddr_in server;

///////////////////// Global Prototypes ///////////////////
// Receiving
void ShowLastErr(bool wsa);
int ServerReceiveSetup();
int ServerListen(HANDLE hFile);
DWORD WINAPI ServerListenThread(LPVOID lpParameter);
void ServerCleanup();
DWORD WINAPI ServerReceiveThread(LPVOID lpParameter);
void CALLBACK ServerCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ServerWriteToFileThread(LPVOID lpParameter);
// Sending
int ServerSendSetup(char* addr);
int ServerSend(HANDLE hFile);
DWORD WINAPI ServerSendThread(LPVOID lpParameter);

#endif
