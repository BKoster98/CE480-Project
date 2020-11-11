// Author : Dr. Foster
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // ascii to integer conversion
#include <unistd.h>   // used for Getopt

#include "portable_socket.h"

CHAR ipaddr[INET_ADDRSTRLEN] = "127.0.0.1";

int main(int argc, char *argv[])
{
    Socket server, client;
    int port = 60481;
    int opt = 0; 
    Message msg;
    char* line[256];
    size_t bytes, size = 0;

	while ((opt = getopt(argc, argv, "p:a:")) != -1){
		switch (opt) {
			case 'p': 
				port = atoi(optarg);
				break;
            case 'a':
                strcpy(ipaddr,optarg);
                break;
			default:
                exit(1);
		}
	}
    printf("The client is using address %s, port %d\n", ipaddr, port);

    initialize_sockets();
    connect_to_server(&server, ipaddr, port);

    while (read_msg(&server, &msg) > 0) {
        printf("%s", msg.buffer);

        if (fgets((char*)line, sizeof(line), stdin)) {
            // strip newline character
            char* pos = (char*)line;
            while (*pos) {
                if (*pos == '\n' || *pos == '\r') {
                    *pos = 0;
                } else {
                    pos += 1;
                }
            }
            send_string(&server, (char*)line);
        } else {
            close_socket(&server);
            break;
        }
    }

    cleanup_sockets();
    return 0;
}
