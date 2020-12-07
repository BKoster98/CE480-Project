#include "portable_socket.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)

static windows_socket_data was;

windows_socket_data::windows_socket_data() {
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "Failed. Winsock error Code : %d\n", WSAGetLastError());
        exit(-1);
    }
}

windows_socket_data::~windows_socket_data() {
    WSACleanup();
}

#endif

socket_error::socket_error(const std::string& msg) : std::runtime_error(msg + "::" + socket_error::socket_error_message()) {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    error_code = WSAGetLastError();
#else
    error_code = errno;
#endif
}

std::string socket_error::socket_error_message() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    auto err_code = WSAGetLastError();
    if (err_code == 0) return "Socket closed";
    char *s = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, err_code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)&s, 0, NULL);
    std::string result{s};
    LocalFree(s);
#else
    if (errno == 0) return "Socket closed";
    char buffer[1024];
    strerror_r(errno, buffer, sizeof(buffer));
    std::string result{buffer};
#endif

    return result;
}


void base_socket::send(const std::string &msg) {
    send_byte((uint8_t) msg.length());
    send_string(msg);
}

void base_socket::send_byte(const uint8_t &byte) {
    socket_error::guard(::send(handle, (const char*) &byte, 1, 0), "socket::send_byte");
}

void base_socket::send_string(const std::string &msg) {
    socket_error::guard(::send(handle, msg.c_str(), (int) msg.size(), 0), "socket::send_string");
}

std::string base_socket::recv() {
    return recv_string(recv_byte());
}

uint8_t base_socket::recv_byte() {
    uint8_t result = 0;
    socket_error::guard(::recv(handle, (char*) &result, 1, 0), "Unable to read byte from socket.");
    return result;
}

std::string base_socket::recv_string(int length) {
    std::vector<char> buffer(length);

    int total_bytes_read = 0;
    while (total_bytes_read != length) {
        total_bytes_read += socket_error::guard(::recv(handle, (char*) &buffer[total_bytes_read], length - total_bytes_read, 0));
    }

    return std::string(buffer.begin(), buffer.end());
}

std::string base_socket::ipv4_addr() {
    char result[INET_ADDRSTRLEN];
    InetNtop(AF_INET, &(addr.sin_addr), result, INET_ADDRSTRLEN);
    return result;
}


base_socket::~base_socket() {
    if (handle != INVALID_SOCKET) {
      std::cout << "Closing socket " << handle << '\n';
      socket_error::nzguard(closesocket(handle), "Closing socket");
    }
}

server_socket::server_socket(in_port_t port, int depth) : base_socket() {
    /* The socket() call takes three arguments. The first is the network protocol "Address Family", hence the AF_prefix.
    The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is usually a
    TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The third parameter is the specific protocol, such as ICMP,
    IGMP, or for the purposes of this chat program, TCP, which uses the constant IPPROTO_TCP. */
    if ((handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        throw socket_error("Could not create socket");
    }
    std::cout << "Opened server_socket: " << handle << '\n';

    // The socket now exists...but it isn't configured with an IP address or port number yet.
    {
        // This block sets up the IP address that the server will listen on (in case is has multiple IP addresses) and the port
        //InetPton(AF_INET, ipv4addr, &socketServer.sin_addr.s_addr); // The variable ip4addr is a character array that has an IPv4 address in it. The code defines it above to
        // to 127.0.0.1, so it will only talk to itself. The  InetPton function converts this text string to a 32-bti number and stores
        // it in the socket socketServer.
        //or allow the system to select one
        // NOTE!!! when you use INADDR_ANY, the host will print 0.0.0.0 as the IP Address, but it is listening on all IP's.
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_family = AF_INET;							// Must agree with the socket Address Family type

        //CE-480 - Change the port number that the server is using by altering the following line.
        addr.sin_port = htons(port);						// htons() converts the host endianness to network endianness

        // This should always be used when transmitting integers
        // ntohs() converts the opposite way for receiving integers.
    }

    if (bind(handle, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        throw socket_error("Could not bind socket");
    }

    // tells socket to start listening for clients, 5 is the arbitrarily chosen number of backlogged connections allowed
    // This only needs to be done once on the server's original socket.
    if (listen(handle, depth) != 0) {
        throw socket_error("Socket listen failed");
    }
}

base_socket server_socket::accept() {
    socklen_t c = sizeof(struct sockaddr_in);

    // This call will block until a client attempts to connect. At that point,
    // a new socket is created (stored in s_new) that is a complete socket between this server
    // and the client. The orginal socket s still existed to accept new calls. Information about the connection,
    // such as the client's IP and port number, is stored to socketClient.
    struct sockaddr_in a{0, 0, 0, {0}, {0}};
    SOCKET h = socket_error::guard(::accept(handle, (struct sockaddr *)&a, &c), "Accept failed");

    return base_socket(h, a);
}

base_socket nonblocking_server_socket::accept() {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(handle, &read_fds);
    struct timeval timeout{0, 0};
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    auto rc = select(int(handle) + 1, &read_fds, nullptr, nullptr, &timeout);

    if (rc < 0) {
        throw socket_error("accept select error");
    } else if (rc == 0) {
        throw std::runtime_error("accept timeout");
    }

    return server_socket::accept();
}

bool nonblocking_server_socket::accept(const std::function<void(base_socket)> &handler, std::chrono::milliseconds delay) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(handle, &read_fds);
    struct timeval timeout{0, 0};
    auto int_sec = std::chrono::duration_cast<std::chrono::seconds>(delay);
    auto int_usec = std::chrono::duration_cast<std::chrono::microseconds>(delay - int_sec);
    timeout.tv_sec = (long) int_sec.count();
    timeout.tv_usec = (long) int_usec.count();

    auto rc = select(int(handle) + 1, &read_fds, nullptr, nullptr, &timeout);
    if (rc < 0) {
        throw socket_error("accept select error");
    }

    if (rc > 0) {
        handler(server_socket::accept());
        return true;
    }

    return false;
}

simple_server::simple_server(in_port_t port, unsigned depth) : nonblocking_server_socket(port, depth), running(false) {
}

void simple_server::stop() {
    running = false;
}

client_socket::client_socket(const std::string &ipaddr, in_port_t port) {
    /* The socket() call takes three arguments. The first is the network protocol "Address Family", hence the AF_prefix.
    The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is usually a
    TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The third parameter is the specific protocol, such as ICMP,
    IGMP, or for the purposes of the program, TCP, which uses the constant IPPROTO_TCP. */
    if ((handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        throw socket_error("Could not create socket");
    }
    std::cout << "Opened client_socket: " << handle << '\n';
    // The socket now exists...but it isn't configured with an IP address or port number yet.

    InetPton(AF_INET, ipaddr.c_str(), &addr.sin_addr.s_addr);   // converts the IP address as a text string in ipv4addr to a number*/
    addr.sin_family = AF_INET;							// Must agree with the socket Address Family type*/

    // htons() converts the host endianness to network endianness
    // This should always be used when transmitting integers
    // ntohs() converts the opposite way for receiving integers.
    addr.sin_port = htons(port);

    //Connect to remote server
    // negative return values indicate an error. For this demo/project, we don't need to troubleshoot, so just "scream and die"
    if (connect(handle, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        throw socket_error("Could not connect socket");
    }
}

