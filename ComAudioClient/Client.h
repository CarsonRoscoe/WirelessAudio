#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <windows.h>
#include "circularbuffer.h"
#include <QBuffer>

///////////////////// Global Prototypes ///////////////////
// Sending Prototypes
void ShowLastErr(bool wsa);
int ClientSendSetup(char* addr);
void ClientCleanup();
int ClientSend(HANDLE hFile);
int ClientSendMicrophoneData(HANDLE hFile);
DWORD WINAPI ClientSendMicrophoneThread(LPVOID lpParameter);
DWORD WINAPI ClientSendThread(LPVOID lpParameter);
void ClientCleanup(SOCKET s);
// Receiving Prototypes
int ClientReceiveSetup();
int ClientListen(HANDLE hFile);
DWORD WINAPI ClientListenThread(LPVOID lpParameter);
DWORD WINAPI ClientReceiveThread(LPVOID lpParameter);
void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter);

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT 7001
#define CLIENT_DEFAULT_PORT 7002
#define FILENAMESIZE        100
#define ERRORSIZE           512
#define CLIENT_PACKET_SIZE  8192
#define SERVER_PACKET_SIZE  8192

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[CLIENT_PACKET_SIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

///////////////////// Global Variables ////////////////////
// Sending
extern char addressSend[100];
extern SOCKET sendSock;
extern struct sockaddr_in server;
extern char errMsg[ERRORSIZE];
extern bool isRecording;
extern QBuffer *microphoneBuffer;
// Receiving
extern SOCKET listenSock, acceptSock;
extern bool listenSockOpen, acceptSockOpen;
extern WSAEVENT acceptEvent;
extern HANDLE hReceiveFile;
extern bool hReceiveOpen;
extern LPSOCKET_INFORMATION SI;
extern CircularBuffer* circularBufferRecv;

#endif
