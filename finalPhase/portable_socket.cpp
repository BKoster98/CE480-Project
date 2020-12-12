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
        total_bytes_read += socket_error::guard(::recv(handle,
                                                (char*) &buffer[total_bytes_read],
                                                length - total_bytes_read, 0));
    }

    return std::string(buffer.begin(), buffer.end());
}
// returns address as a string
std::string base_socket::ipv4_addr() {
    char result[INET_ADDRSTRLEN];
    InetNtop(AF_INET, &(addr.sin_addr), result, INET_ADDRSTRLEN);
    return result;
}


base_socket::~base_socket() {
    if (handle != INVALID_SOCKET) {
        socket_error::nzguard(::closesocket(handle), "Closing socket");
    }
}

server_socket::server_socket(in_port_t port, int depth) : base_socket() {
    /* The socket() call takes three arguments. The first is the network 
     * protocol "Address Family", hence the AF_prefix.
     * The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. 
     * The next asks for the port type, which is usually a TCP port with 
     * SOCK_STREAM, or a UDP port with SOCK_DGRAM. 
     * The third parameter is the specific protocol, such as ICMP, IGMP, 
     * or for the purposes of this chat program, TCP, which uses 
     * the constant IPPROTO_TCP. 
     */
    if ((handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        throw socket_error("Could not create socket");
    }

    // The socket now exists...but it isn't configured with an IP address or port number yet.
    {
        // This block sets up the IP address that the server will listen on 
        // NOTE!!! when you use INADDR_ANY, the host will print 0.0.0.0 
        // as the IP Address, but it is listening on all IP's.
        addr.sin_addr.s_addr = INADDR_ANY;

        // Must agree with the socket Address Family type
        addr.sin_family = AF_INET;							

        // Change the port number that the server is using by altering the following line.
        // htons() converts the host endianness to network endianness
        addr.sin_port = htons(port);						

        // This should always be used when transmitting integers
        // ntohs() converts the opposite way for receiving integers.
    }

    if (bind(handle, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        throw socket_error("Could not bind socket");
    }

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
    struct sockaddr_in a { 0, 0, 0, {0}, {0} };
    SOCKET h = socket_error::guard(::accept(handle, (struct sockaddr *)&a, &c), "Accept failed");

    return base_socket(h, a);
}

client_socket::client_socket(const std::string &ipaddr, in_port_t port) {
    /* The socket() call takes three arguments. The first is the network protocol 
     * "Address Family", hence the AF_prefix. The two most common are AF_INET for 
     * IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is 
     * usually a TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The 
     * third parameter is the specific protocol, such as ICMP, IGMP, or for the 
     * purposes of the program, TCP, which uses the constant IPPROTO_TCP. 
     */
    if ((handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        throw socket_error("Could not create socket");
    }

    // The socket now exists...but it isn't configured with an IP address or port number yet.
    // converts the IP address as a text string in ipv4addr to a number
    InetPton(AF_INET, ipaddr.c_str(), &addr.sin_addr.s_addr);   
    
    // Must agree with the socket Address Family type
    addr.sin_family = AF_INET;							

    // htons() converts the host endianness to network endianness
    // This should always be used when transmitting integers
    // ntohs() converts the opposite way for receiving integers.
    addr.sin_port = htons(port);

    // Connect to remote server
    // negative return values indicate an error. 
    if (connect(handle, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        throw socket_error("Could not connect socket");
    }
}

