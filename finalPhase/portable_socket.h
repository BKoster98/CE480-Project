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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
class windows_socket_data {
public:
    windows_socket_data();
    ~windows_socket_data();
private:
    static windows_socket_data was;
    WSADATA wsa;
};
#endif

class socket_error : public std::runtime_error {
public:
    explicit socket_error(const std::string& msg);
    static std::string socket_error_message();

    int get_error_code() const {
        return error_code;
    }

    template<typename T>
    static T guard(T expr, const std::string& msg="") {
        // if (expr ==  0) throw std::runtime_error("socket closed");
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
    base_socket(SOCKET _handle, struct sockaddr_in _addr) : handle(_handle), addr(_addr) {
      // std::cout << "Opened socket: " << handle << '\n';
    }
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

    void non_blocking() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        u_long mode = 1;
        socket_error::nzguard(ioctlsocket(handle, FIONBIO, &mode), "ioctlsocket failed");
#else
        auto flags = socket_error::guard(fcntl(handle, F_GETFL), "Unable to retrieve flags");
        socket_error::nzguard(fcntl(handle, F_SETFL, flags | O_NONBLOCK), "Unable to make socket nonblocking");
#endif
    }

    void blocking() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        u_long mode = 0;
        socket_error::nzguard(ioctlsocket(handle, FIONBIO, &mode), "ioctlsocket failed");
#else
        auto flags = socket_error::guard(fcntl(handle, F_GETFL), "Unable to retrieve flags");
        socket_error::nzguard(fcntl(handle, F_SETFL, flags & (~O_NONBLOCK)), "Unable to make socket blocking");
#endif
    }

protected:
    base_socket() : handle(INVALID_SOCKET), addr{ 0, 0, 0, {0}, {0} } {}

    virtual void send_byte(const uint8_t &byte);
    virtual void send_string(const std::string &msg);
    virtual uint8_t recv_byte();
    virtual std::string recv_string(int length);

    SOCKET handle;
    struct sockaddr_in addr;
};

class server_socket : public base_socket {
public:
    explicit server_socket(in_port_t port, int depth = 10);
    virtual base_socket accept();
};

class nonblocking_server_socket : public server_socket {
public:
    explicit nonblocking_server_socket(in_port_t port, unsigned depth = 10) : server_socket(port, depth) {
        non_blocking();
    }

    base_socket accept() override;
    virtual bool accept(const std::function<void(base_socket)> &handler, std::chrono::milliseconds delay = std::chrono::milliseconds(100));
};

class simple_server : public nonblocking_server_socket {
public:
    explicit simple_server(in_port_t port, unsigned depth = 10);

    template<typename LAMBDA>
    void start(LAMBDA lambda);

    void stop();

private:
    std::atomic_bool running;
};

class server_exit { };

//starts up thread and runs the function
template<typename LAMBDA>
void simple_server::start(LAMBDA lambda) {
    using namespace std::chrono_literals;
    std::vector<std::future<std::pair<std::string,bool>>> jobs;
    running = true;

    while (running) {
        accept([&jobs, &lambda](base_socket client) {
            //std::cout << "Client (" << client.ipv4_addr() << ") connected\n";
            client.blocking();
            std::packaged_task<std::pair<std::string,bool>(base_socket)> task{lambda};
            jobs.push_back(task.get_future());
            std::thread job(std::move(task), std::move(client));
            job.detach();
        });

        for (auto job = jobs.begin(); job != jobs.end();) {

            if (job->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                try {
                    auto status = job->get();
                    //std::cout << "Client (" << status.first << ") exited: " << status.second << "\n";

                } catch(server_exit) {
                    running = false;
                } catch(std::exception& err) {
                    //std::cout << "Client exited with exception: " << err.what() << "\n";
                }
                job = jobs.erase(job);
            } else {
                ++job;
            }
        }
    }
    std::cout << "Server shutting down\n";
}

class client_socket : public base_socket {
public:
    client_socket(const std::string &ipaddr, in_port_t port);
};

