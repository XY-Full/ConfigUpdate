#pragma once

#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <errno.h>
    typedef int socket_t;
#endif

class SocketWrapper 
{
public:
    explicit SocketWrapper(socket_t fd);
    ~SocketWrapper();

    socket_t fd() const;
    void close();

    bool sendAll(const std::string& data);
    bool recvAll(std::string& out, size_t size, bool use_peek = false);

private:
    socket_t sock_fd_ = -1;
};
