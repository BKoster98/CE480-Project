// Author : Allison Hurley and Ben Kocik
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // ascii to integer 
//#include <unistd.h>   // used for Getopt

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

    for (int i = 1; i < argc; i++) {
        if (argv[i] != ' '){
            char* str = argv[i];
            switch (str[1]) {
                case 'p':
                    port = atoi(str);
                    break;
                case 'a':
                    ipaddr[i] = str;
                    break;
                default:
                    exit(1);
            }
        }
    }

    //for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
    //    switch (argv[optind][1]) {
    //        case 'p':
    //            port = atoi(argv[optind][2]);
    //        case 'a':
    //            
    //            strcpy_s(ipaddr,argv[optind][2]);
    //        default:
    //            exit(1);
    //    }
    //}

	//while ((opt = getopt(argc, argv, "p:a:")) != -1){
	//	switch (opt) {
	//		case 'p': 
	//			port = atoi(optarg);
	//			break;
 //           case 'a':
 //               strcpy(ipaddr,optarg);
 //               break;
	//		default:
 //               exit(1);
	//	}
	//}

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
