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
// Control Channel
int ClientSendRequest(int flag);
DWORD WINAPI ClientControlThreadSend(LPVOID lpParameter);

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT 7001
#define CLIENT_DEFAULT_PORT 7002
#define P2P_DEFAULT_PORT    7003
#define CONTROL_PORT        7004
#define FILENAMESIZE        100
#define ERRORSIZE           512
#define CLIENT_PACKET_SIZE  8192
#define SERVER_PACKET_SIZE  8192

////////////////// Control Channel Flags ///////////////////
#define GET_UPDATE_SONG_LIST    1
#define SEND_SONG_TO_SERVER     2
#define GET_SONG_FROM_SERVER    3

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
extern char address[100];
extern SOCKET sendSock;
extern bool sendSockOpen;
extern struct sockaddr_in server;
extern char errMsg[ERRORSIZE];
extern bool isRecording;
extern QBuffer *microphoneBuffer, *listeningBuffer;
// Receiving
extern SOCKET listenSock, acceptSock;
extern bool listenSockOpen, acceptSockOpen;
extern WSAEVENT acceptEvent;
extern HANDLE hReceiveFile;
extern bool hReceiveOpen;
extern LPSOCKET_INFORMATION SI;
extern CircularBuffer* circularBufferRecv;
// P2P
extern char p2pAddress[100];
extern struct sockaddr_in otherClient;
extern SOCKET p2pListenSock, p2pAcceptSock, p2pSendSock;
extern bool p2pListenSockOpen, p2pAcceptSockOpen, p2pSendSockOpen;
extern WSAEVENT p2pAcceptEvent;
extern LPSOCKET_INFORMATION p2pSI;
// Control Channel
extern char sendFileName[100], recvFileName[100];
extern SOCKET controlSock;
extern bool controlSockOpen;

#endif
