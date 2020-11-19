#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

// This pragma ONLY works in Visual Studio.
// You will need to manually link this library when using something else.
#pragma comment(lib,"ws2_32.lib") //Winsock Library - don't touch this.

WSADATA wsa;

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SOCKET int
#define CHAR char
#define PSTR CHAR*
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define InetPton inet_pton
#define InetNtop inet_ntop
#define sscanf_s sscanf
#define sprintf_s snprintf
#define Sleep sleep
#define closesocket close
#define WSACleanup() (0)

int wsa;
#define WSAStartup(x,y) (0)
#define WSAGetLastError() (errno)
#endif

// create some constants for error codes if the program dies... helps figure out where things went wrong.
enum {
    ERROR_WINSOCK_INIT_FAILURE = 1,
    ERROR_WINSOCK_SOCKET_CREATE_FAILURE,
    ERROR_WINSOCK_SOCKET_CONNECT_FAILURE,
    ERROR_WINSOCK_SOCKET_SEND_FAILURE,
    ERROR_WINSOCK_SOCKET_READ_FAILURE,
    ERROR_WINSOCK_SOCKET_BIND_FAILURE,
    ERROR_WINSOCK_SOCKET_LISTEN_FAILURE
};

typedef struct {
    SOCKET socket;
    struct sockaddr_in addr;   // sockets are used to access the network
} Socket;

typedef struct {
    char buffer[256];
    unsigned char length;
} Message;

void initialize_sockets();
void initialize_server(Socket* server, unsigned port);
char* ipv4addr(Socket* socket);

int connect_to_server(Socket* server, const char* ipaddr, int port);

int wait_for_client(Socket* server, Socket* client);
int read_msg(Socket* client, Message* msg);
int send_msg(Socket* client, Message* msg);
int send_string(Socket* client, const char* str);

void close_socket(Socket* server);
void cleanup_sockets();

