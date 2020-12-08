// Author : Allison Hurley & Ben Kocik
// Purpose : 
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/
//  Reference : http://beej.us/guide/bgnet/ has a decent explanation of the concepts, just be warned that the sample
//  code is intended for Linux and needs occasional tweaks to run on Windows.

#include <iostream>
#include <regex>
#include "portable_socket.h"
#include "simple_args.h"

std::pair<std::string,bool> random_die_client(base_socket client) {
    try {
        bool running = true; 
        while(running) {
            std::string cmd = client.recv();
            if (cmd == "Q"){
                running = false; 
            }else{
                int randNum = rand()%(std::stoi(cmd)) + 1;
                client.send(std::to_string(randNum));
            }
            
        }
        
    } catch (socket_error& err) {
        //socket errors
        std::cerr << "socket_error in process_client: " << err.what() << "\n";
        //returns IP and error status 
        return std::pair(client.ipv4_addr(),false);
    }

    return std::pair(client.ipv4_addr(),true);
}

int main(int argc, const char** argv)
try {
    simple_args args(argc, argv);
    //picks a port
    in_port_t port = stoi(args("p", "60481"));
    //creates server connection
    simple_server server(port);

    std::cout << "Server started on port " << port << "\n";
    //starts a threaded server for every new connection starts a new thread and runs the function
    server.start(random_die_client);

    return 0;
} catch (socket_error& err) {
    std::cerr << "Socket error: " << err.what() << "\n";
} catch (std::exception& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return -1;
}

