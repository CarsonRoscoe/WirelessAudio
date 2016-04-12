#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <windows.h>
#include "circularbuffer.h"
#include <QBuffer>

///////////////////// Global Prototypes ///////////////////
// Sending Prototypes
void ShowLastErr(bool wsa);
int ClientSendSetup(char* addr, SOCKET sock, int port);
void ClientCleanup();
int ClientSend(HANDLE hFile);
DWORD WINAPI ClientSendMicrophoneThread(LPVOID lpParameter);
DWORD WINAPI ClientSendThread(LPVOID lpParameter);
void ClientCleanup(SOCKET s);
// Receiving Prototypes
int ClientReceiveSetup(SOCKET sock, int port, WSAEVENT event);
int ClientListen(HANDLE hFile);
DWORD WINAPI ClientListenThread(LPVOID lpParameter);
DWORD WINAPI ClientReceiveThread(LPVOID lpParameter);
void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter);
//P2P
int ClientSendMicrophoneData();
int ClientListenP2P();
DWORD WINAPI ClientListenThreadP2P(LPVOID lpParameter);
DWORD WINAPI ClientReceiveThreadP2P(LPVOID lpParameter);
DWORD WINAPI ClientWriteToFileThreadP2P(LPVOID lpParameter);
void CALLBACK ClientCallbackP2P(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT 7001
#define CLIENT_DEFAULT_PORT 7002
#define P2P_DEFAULT_PORT    7003
#define SONG_REQUEST_PORT   7004
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
extern char address[100], p2pAddress[100];
extern SOCKET sendSock, p2pSendSock;
extern bool sendSockOpen, p2pSendSockOpen;
extern struct sockaddr_in server, otherClient;
extern char errMsg[ERRORSIZE];
extern bool isRecording;
extern QBuffer *microphoneBuffer, *listeningBuffer;
// Receiving
extern SOCKET listenSock, acceptSock, p2pListenSock, p2pAcceptSock;
extern bool listenSockOpen, acceptSockOpen, p2pListenSockOpen, p2pAcceptSockOpen;
extern WSAEVENT acceptEvent, p2pAcceptEvent;
extern HANDLE hReceiveFile;
extern bool hReceiveOpen;
extern LPSOCKET_INFORMATION SI, p2pSI;
extern CircularBuffer* circularBufferRecv;

#endif
