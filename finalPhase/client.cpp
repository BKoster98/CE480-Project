// Author : Allison Hurley & Ben Kocik
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <iostream>
#include "portable_socket.h"
#include "simple_args.h"


int main(int argc, const char** argv)
try {
    simple_args args(argc, argv);
    //default value 60481 otherwise inputted port
    in_port_t port = stoi(args("p", "60481"));
    //default value 127.0.0.1 unless inputted address
    std::string ipaddr = args("a", "127.0.0.1");

    //connect to server
    client_socket client(ipaddr, port);
    bool running = true;
    std::string cmd;
    while (running) {
        std::cout << "Enter the number of dice to roll and their sides or Q to quit: ";
        std::getline(std::cin, cmd);

        if(cmd == "Q"){
            running = false; 
        } else if (cmd == "iop") {
            // sends string to server
            client.send(cmd);
            // gets string back from server and prints
            std::cout << ">> " << client.recv() << '\n';
        }else {
            std::cout << "Unknown command";
        }
    }
    // ends program if general socket error or connection error 
} catch(socket_error& err) {
    std::cerr << "Err: " << err.what() << "\n";
} catch(std::exception&) {
    std::cerr << "Connection to server closed\n";
}

