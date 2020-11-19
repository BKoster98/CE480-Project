// Author : Allison Hurley and ???
// Purpose : Complete requirements for Phase 2
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/
//  Reference : http://beej.us/guide/bgnet/ has a decent explanation of the concepts, just be warned that the sample
//  code is intended for Linux and needs occasional tweaks to run on Windows.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>   // ascii to integer conversion

#include "portable_socket.h"

enum {
    EXIT_FLAG = 1,
    CLOSE_FLAG,
    CLIENT_DISCONNECT
};

int ascii_hex_to_int(char ch) {
    ch = tolower(ch);
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;

    // should never happen...
    return 0;
}

char hex_to_ascii(int v) {
    if (v >= 0 && v <= 9) return '0' + v;
    if (v >= 10 && v <= 15) return 'a' + (v - 10);

    // should never happen...
    return ' ';
}

const char* convert_mac(const char* mac) {
    // not a thread safe approach...
    static char result[256];

    if (strlen(mac) != 17) {
        return 0;
    }

    result[ 0] = tolower(mac[0]);
    result[ 1] = hex_to_ascii(ascii_hex_to_int(mac[1]) | 0x2);
    result[ 2] = tolower(mac[3]);
    result[ 3] = tolower(mac[4]);
    result[ 4] = ':';

    result[ 5] = tolower(mac[6]);
    result[ 6] = tolower(mac[7]);
    result[ 7] = 'f';
    result[ 8] = 'f';
    result[ 9] = ':';

    result[10] = 'f';
    result[11] = 'e';
    result[12] = tolower(mac[9]);
    result[13] = tolower(mac[10]);
    result[14] = ':';

    result[15] = tolower(mac[12]);
    result[16] = tolower(mac[13]);
    result[17] = tolower(mac[15]);
    result[18] = tolower(mac[16]);
    result[19] = 0;

    return result;
}

// poor man's optarg
const char* my_optarg = 0;

char my_getopt(int argc, const char** argv, const char* format) {
    static int count = 1;

    if (count >= argc) return -1;

    ++count;
    if (*argv[count] == '-' || *argv[count] == '/') {
        my_optarg = argv[count + 1];
        return argv[count][1];
    }

    return -1;
}

int process_client(Socket* client) {
    char resp[255] = "";
    Message msg;
    int rc = 0;

    while(rc == 0) {
        msg.length = snprintf(msg.buffer, sizeof(msg.buffer), "%sEnter a MAC Address, Q quit, or T to terminate server: ", resp);
        send_msg(client, &msg);
        read_msg(client, &msg);
        if (msg.length == 1) {
            if (tolower(*msg.buffer) == 't') {
                printf("Shutting Down\n");
                rc = EXIT_FLAG;
            }
            if (tolower(*msg.buffer) == 'q') {
                printf("Terminate Client\n");
                rc = CLOSE_FLAG;
            } else {
                snprintf(resp, sizeof(resp), "Unknown command '%c'\n", *msg.buffer);
            }
        } else if (msg.length > 0) {

            // is this a MAC address? do something useful...
            const char* ipv6 = convert_mac(msg.buffer);
            if (ipv6) {
                snprintf(resp, sizeof(resp), "The EUI-64 number is %s.\n", ipv6);
            } else {
                resp[0] = 0;
                snprintf(resp, sizeof(resp), "Unable to process '%s'.\n", msg.buffer);
                printf("Unable to process >%s<\n", msg.buffer);
            }

        } else {
            printf("Client disconnect\n");
            rc = CLIENT_DISCONNECT;
        }
    }

    close_socket(client);
    return rc;
}

int main(int argc, const char *argv[])
{
    // todo use argc/argv to get optional port command....
    Socket server, client;
    int opt = 0;
    int port = 60481;

    while ((opt = my_getopt(argc, argv, "p:")) != -1){
      switch (opt) {
        case 'p': 
          port = atoi(my_optarg);
          break;
        default:
          exit(1);
      }
    }

    initialize_sockets();
    initialize_server(&server, port);

    while (wait_for_client(&server, &client) == 0) {
        printf("Connection established with %s\n", ipv4addr(&client));
        if (process_client(&client) == EXIT_FLAG) break;
    }

    cleanup_sockets();

    return 0;
}

