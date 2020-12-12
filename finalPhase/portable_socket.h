#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <thread>
#include <future>
#include <chrono>
#include <iostream>
#include <fcntl.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

typedef USHORT in_port_t;

// This pragma ONLY works in Visual Studio.
// You will need to manually link this library when using something else.
#pragma comment(lib,"ws2_32.lib") //Winsock Library - don't touch this.

class windows_socket_data {
public:
    windows_socket_data();
    ~windows_socket_data();
private:
    static windows_socket_data was;
    WSADATA wsa;
};

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <arpa/inet.h>
#include <unistd.h>


#define SOCKET int
#define CHAR char
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define InetPton inet_pton
#define InetNtop inet_ntop
#define closesocket close
#endif

class socket_closed : public std::runtime_error {
public:
    explicit socket_closed() : std::runtime_error("socket closed") {}
};

class socket_error : public std::runtime_error {
public:
    explicit socket_error(const std::string& msg);
    static std::string socket_error_message();

    int get_error_code() const {
        return error_code;
    }

    template<typename T>
    static T guard(T expr, const std::string& msg="") {
        if (expr ==  0) throw socket_closed();
        if (expr == -1) throw socket_error(msg);
        return expr;
    }

    template<typename T>
    static T nzguard(T expr, const std::string& msg="") {
        if (expr == -1) throw socket_error(msg);
        return expr;
    }

private:
    int error_code;
};

class base_socket {
public:
    base_socket(SOCKET _handle, struct sockaddr_in _addr) : handle(_handle), addr(_addr) { }
    virtual ~base_socket();

    // Non-copyable.
    base_socket(const base_socket&) =delete;
    base_socket& operator=(const base_socket&) =delete;

    // Moveable
    base_socket(base_socket&& sock) noexcept : handle(sock.handle), addr(sock.addr) {
        sock.handle = INVALID_SOCKET;
    }

    virtual void send(const std::string &msg);
    virtual std::string recv();
    virtual std::string ipv4_addr();

protected:
    base_socket() : handle(INVALID_SOCKET), addr{ 0, 0, 0, {0}, {0} } {}

    virtual void send_byte(const uint8_t &byte);
    virtual void send_string(const std::string &msg);
    virtual uint8_t recv_byte();
    virtual std::string recv_string(int length);

    SOCKET handle;
    struct sockaddr_in addr;
};

class client_socket : public base_socket {
public:
    client_socket(const std::string &ipaddr, in_port_t port);
};

class server_socket : public base_socket {
public:
    explicit server_socket(in_port_t port, int depth = 10);
    virtual base_socket accept();
};

class simple_server : public server_socket {
public:
    explicit simple_server(in_port_t _port, unsigned _depth = 10) :
        server_socket(_port, _depth) {}

    template<typename LAMBDA>
    void start(LAMBDA lambda);
};

// creates threads and runs client function
template<typename LAMBDA>
void simple_server::start(LAMBDA lambda) {
    while (true) {
        //waits for a client
        auto client{accept()};

        // creates thread and passes it the client socket and the function
        auto worker = std::thread([client=std::move(client), lambda] () mutable {
                auto ipaddr = client.ipv4_addr();
                std::cout << "Client started from " << ipaddr << '\n';
                // runs function
                lambda(std::move(client));
                std::cout << "Client from " << ipaddr << " exited\n";
                });
        // forgets about thread once created and continues to look for more clients
        worker.detach();
    }
}

