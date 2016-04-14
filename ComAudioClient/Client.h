#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <windows.h>
#include "circularbuffer.h"
#include "audiomanager.h"
#include "win32communicationworker.h"
#include <QBuffer>
#include <iostream>
#include <fstream>
#include <QAudioInput>

///////////////////// Global Prototypes ///////////////////
// Sending Prototypes
void ShowLastErr(bool wsa);
int ClientSendSetup(char* addr, SOCKET &sock, int port);
void ClientCleanup();
int ClientSend(HANDLE hFile);
DWORD WINAPI ClientSendMicrophoneThread(LPVOID lpParameter);
DWORD WINAPI ClientSendThread(LPVOID lpParameter);
void ClientCleanup(SOCKET s);
// Receiving Prototypes
int ClientReceiveSetup(SOCKET &sock, int port, WSAEVENT &event);
int ClientListen();
DWORD WINAPI ClientListenThread(LPVOID lpParameter);
DWORD WINAPI ClientReceiveThread(LPVOID lpParameter);
void CALLBACK ClientCallback(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientWriteToFileThread(LPVOID lpParameter);
//P2P
int ClientReceiveSetupP2P();
int ClientSendSetupP2P(char* addr);
int ClientSendMicrophoneData();
int ClientListenP2P();
void CleanupRecvP2P();
void CleanupSendP2P();
DWORD WINAPI ClientListenThreadP2P(LPVOID lpParameter);
DWORD WINAPI ClientReceiveThreadP2P(LPVOID lpParameter);
void CALLBACK ClientCallbackP2P(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
// Control Channel
int ClientSendRequest(int flag);
DWORD WINAPI ClientControlThreadSend(LPVOID lpParameter);
extern void *app;

///////////////////// Macros //////////////////////////////
#define SERVER_DEFAULT_PORT 7001
#define CLIENT_DEFAULT_PORT 7002
#define P2P_DEFAULT_PORT    7003
#define CONTROL_PORT        7004
#define MULTICAST_PORT      4985
#define MULTICAST_IP        "234.5.6.7"
#define FILENAMESIZE        100
#define ERRORSIZE           512
#define CLIENT_PACKET_SIZE  25600
#define SERVER_PACKET_SIZE  25600
#define CONTROL_PACKET_SIZE 256

////////////////// Control Channel Flags ///////////////////
#define GET_UPDATE_SONG_LIST    1
#define SEND_SONG_TO_SERVER     2
#define GET_SONG_FROM_SERVER    3

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[SERVER_PACKET_SIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

///////////////////// Global Variables ////////////////////
// Sending
extern char address[100];
extern SOCKET sendSock;
extern bool sendSockClosed;
extern HANDLE hSendFile;
extern bool hSendClosed;
extern struct sockaddr_in server;
extern char errMsg[ERRORSIZE];
extern bool isRecording;
extern QBuffer *microphoneBuffer, *listeningBuffer;
extern CircularBuffer * micBuf;
// Receiving
extern SOCKET listenSock, acceptSock;
extern bool listenSockClosed, acceptSockClosed;
extern WSAEVENT acceptEvent;
extern HANDLE hReceiveFile;
extern bool hReceiveClosed;
extern LPSOCKET_INFORMATION SI;
extern CircularBuffer* circularBufferRecv;
// P2P
extern char p2pAddress[100];
extern struct sockaddr_in otherClient;
extern SOCKET p2pListenSock, p2pAcceptSock, p2pSendSock;
extern bool p2pListenSockClosed, p2pAcceptSockClosed, p2pSendSockClosed;
extern WSAEVENT p2pAcceptEvent;
extern LPSOCKET_INFORMATION p2pSI;
// Control Channel
extern char sendFileName[100], recvFileName[100];
extern char **songList;
extern SOCKET controlSock;
extern bool controlSockClosed;

extern QAudioInput * audio;
extern int packetcounter;

#endif
