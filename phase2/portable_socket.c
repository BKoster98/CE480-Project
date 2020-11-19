#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portable_socket.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
WSADATA wsa;
#endif

void initialize_sockets() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    // Before using Winsock calls on Windows, the Winsock library needs to be initialized...
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "Failed. Winsock error Code : %d\n", WSAGetLastError());
        exit(ERROR_WINSOCK_INIT_FAILURE);
    }
#endif
}

void initialize_server(Socket* server, unsigned port) {
    /* The socket() call takes three arguments. The first is the network protocol "Address Family", hence the AF_prefix.
    The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is usually a
    TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The third parameter is the specific protocol, such as ICMP,
    IGMP, or for the purposes of this chat program, TCP, which uses the constant IPPROTO_TCP. */
    if ((server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Could not create socket : %d\n", WSAGetLastError());
        exit(ERROR_WINSOCK_SOCKET_CREATE_FAILURE);
    }

    // The socket now exists...but it isn't configured with an IP address or port number yet.
    {
        // This block sets up the IP address that the server will listen on (in case is has multiple IP addresses) and the port
        //InetPton(AF_INET, ipv4addr, &socketServer.sin_addr.s_addr); // The variable ip4addr is a character array that has an IPv4 address in it. The code defines it above to
        // to 127.0.0.1, so it will only talk to itself. The  InetPton function converts this text string to a 32-bti number and stores
        // it in the socket socketServer.
        //or allow the system to select one
        server->addr.sin_addr.s_addr = INADDR_ANY;					// NOTE!!! when you use INADDR_ANY, the host will print 0.0.0.0 as the IP Address, but it is listening on all IP's.
        server->addr.sin_family = AF_INET;							// Must agree with the socket Address Family type

        //CE-480 - Change the port number that the server is using by altering the following line.
        server->addr.sin_port = htons(port);						// htons() converts the host endianness to network endianness

        // This should always be used when transmitting integers
        // ntohs() converts the opposite way for receiving integers.
    }

    if (bind(server->socket, (struct sockaddr *) &server->addr, sizeof(server->addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Could not bind socket\n");
        exit(ERROR_WINSOCK_SOCKET_BIND_FAILURE);
    }

    // tells socket to start listening for clients, 5 is the arbitrarily chosen number of backlogged connections allowed
    // This only needs to be done once on the server's original socket.
    if (listen(server->socket, 5) != 0) {
        exit(ERROR_WINSOCK_SOCKET_LISTEN_FAILURE);
    }
}


int wait_for_client(Socket* server, Socket* client) {
    socklen_t c = sizeof(struct sockaddr_in);

    // This call will block until a client attempts to connect. At that point,
    // a new socket is created (stored in s_new) that is a complete socket between this server
    // and the client. The orginal socket s still existed to accept new calls. Information about the connection,
    // such as the client's IP and port number, is stored to socketClient.
    client->socket = accept(server->socket, (struct sockaddr *)&client->addr, &c);
    if (client->socket == INVALID_SOCKET)
    {
        fprintf(stderr, "Accept failed with error code : %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

int connect_to_server(Socket* server, const char* ipaddr, int port) {
    /* The socket() call takes three arguments. The first is the network protocol "Address Family", hence the AF_prefix.
    The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is usually a
    TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The third parameter is the specific protocol, such as ICMP,
    IGMP, or for the purposes of the program, TCP, which uses the constant IPPROTO_TCP. */
    if ((server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Could not create socket : %d\n", WSAGetLastError());
        exit(ERROR_WINSOCK_SOCKET_CREATE_FAILURE);
    }
    // The socket now exists...but it isn't configured with an IP address or port number yet.

    InetPton(AF_INET, ipaddr, &server->addr.sin_addr);   // converts the IP address as a text string in ipv4addr to a number
    server->addr.sin_family = AF_INET;							// Must agree with the socket Address Family type

    // htons() converts the host endianness to network endianness
    // This should always be used when transmitting integers
    // ntohs() converts the opposite way for receiving integers.
    server->addr.sin_port = htons(port);

    {
        /*InetNtop(AF_INET, &(addrServer.sin_addr), (PSTR)ipv4addr, INET_ADDRSTRLEN);   // converts the IP address as a 32-bit number to a printable character array*/
        /*printf("Attempting to connect to %s on port %d...\n", ipv4addr, ntohs(addrServer.sin_port));*/
    }

    //Connect to remote server
    // negative return values indicate an error. For this demo/project, we don't need to troubleshoot, so just "scream and die"
    if (connect(server->socket, (struct sockaddr *) &server->addr, sizeof(server->addr)) < 0)
    {
        fprintf(stderr, "Could not connect socket %d\n", WSAGetLastError());
        exit(ERROR_WINSOCK_SOCKET_CONNECT_FAILURE);
    }

    return 0;
}

char* ipv4addr(Socket* socket) {
    static char result[INET_ADDRSTRLEN];

    InetNtop(AF_INET, &(socket->addr.sin_addr), result, INET_ADDRSTRLEN);

    return result;
}

int read_msg(Socket* client, Message* msg) {
    int bytesRead = 0;
    char* bufpos = msg->buffer;

    int bytes = recv(client->socket, &msg->length, 1, 0);  // blocks here until something is received on the socket.

    if (bytes != 1) {
        fprintf(stderr, "Ending the session...\n");
        msg->length = 0;
        *msg->buffer = 0;
        return -1;
    }

    bytesRead = 0;
    while (bytesRead != msg->length) {
        bytes = recv(client->socket, bufpos, msg->length - bytesRead, 0);
        if (bytes < 0) {
            fprintf(stderr, "Error reading message buffer\n");
            msg->length = 0;
            *msg->buffer = 0;
            return -1;
        }
        bufpos += bytes;
        bytesRead += bytes;
    }

    // ensure null termination
    msg->buffer[msg->length] = 0;
    return bytesRead;
}

int send_msg(Socket* client, Message* msg) {
    // We sent exactly what they requested...
    if (msg->length == 0) return 0;

    if (send(client->socket, &msg->length, 1, 0) < 0) {
        fprintf(stderr, "Error sending message length\n");
        exit(ERROR_WINSOCK_SOCKET_SEND_FAILURE);
    }

    if (send(client->socket, msg->buffer, msg->length, 0) < 0) {
        fprintf(stderr, "Error sending message buffer\n");
        exit(ERROR_WINSOCK_SOCKET_SEND_FAILURE);
    }

    return 0;
}

int send_string(Socket* client, const char* str) {
    Message msg;
    size_t msglen = strlen(str);

    if (msglen < 256) {
        msg.length = (unsigned char) msglen;
        strncpy(msg.buffer, str, sizeof(msg.buffer));
        return send_msg(client, &msg);
    }

    return -1;
}

void close_socket(Socket* s) {
    closesocket(s->socket);
}

void cleanup_sockets() {
    WSACleanup();
}


