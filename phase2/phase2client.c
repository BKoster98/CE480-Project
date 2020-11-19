// Author : Allison Hurley and Ben Kocik
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // ascii to integer conversion

#include "portable_socket.h"

CHAR ipaddr[INET_ADDRSTRLEN] = "127.0.0.1";

// poor man's optarg
const char* my_optarg = 0;

char my_getopt(int argc, const char** argv, const char* format) {
    static int count = 0;

    ++count;
    if (count >= argc) return -1;

    if (*argv[count] == '-' || *argv[count] == '/') {
        my_optarg = argv[count + 1];
        return argv[count++][1];
        }
        
    return -1;
}

int main(int argc, const char *argv[])
{
    Socket server;
    int port = 60481;
    int opt = 0;
    Message msg;
    char line[256];
    size_t size = 0;

    while ((opt = my_getopt(argc, argv, "a:p:")) != -1){
      switch (opt) {
        case 'a': 
          strncpy(ipaddr, my_optarg, sizeof(ipaddr));
          break;
        case 'p': 
          port = atoi(my_optarg);
          break;
        default:
          exit(1);
      }
    }

    printf("The client is using address %s, port %i\n", ipaddr, port);

    initialize_sockets();
    connect_to_server(&server, ipaddr, port);

    while (read_msg(&server, &msg) > 0) {
        printf("%s", msg.buffer);

        if (fgets((char*) line, sizeof(line), stdin)) {
            // strip newline character
            char* pos = (char*) line;
            while (*pos != 0) {
              if (*pos == '\n' || *pos == '\r') {
                *pos = 0;
              } else {
                pos += 1;
              }
            }
            send_string(&server, line);
        } else {
            close_socket(&server);
            break;
        }
    }

    cleanup_sockets();
    return 0;
}
