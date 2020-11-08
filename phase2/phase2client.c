// Author : Dr. Foster
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <stdio.h>
#include <string.h>

#include "portable_socket.h"

CHAR ipaddr[INET_ADDRSTRLEN] = "127.0.0.1";

int main(int argc, char *argv[])
{
    Socket server, client;
    int port = 60481;
    Message msg;
    char* line = NULL;
    size_t bytes, size = 0;

    initialize_sockets();
    connect_to_server(&server, ipaddr, port);

    while (read_msg(&server, &msg) > 0) {
        printf("%s", msg.buffer);

        if ((bytes = getline(&line, &size, stdin)) > 0) {
            line[bytes - 1] = 0;
            send_string(&server, line);
        } else {
            close_socket(&server);
            break;
        }
    }

    cleanup_sockets();
    return 0;
}
