#include "SocketWrapper.h"
#include <sys/socket.h>
#include <cstring>

SocketWrapper::SocketWrapper(int fd) : sock_fd_(fd) {}

SocketWrapper::~SocketWrapper() 
{
    close();
}

int SocketWrapper::fd() const 
{
    return sock_fd_;
}

void SocketWrapper::close() 
{
    if (sock_fd_ != -1) {
        ::close(sock_fd_);
        sock_fd_ = -1;
    }
}

bool SocketWrapper::sendAll(const std::string& data) 
{
    size_t total_sent = 0;
    while (total_sent < data.size()) 
    {
        ssize_t sent = ::send(sock_fd_, data.data() + total_sent, data.size() - total_sent, 0);
        if (sent <= 0) return false;
        total_sent += sent;
    }
    return true;
}

bool SocketWrapper::recvAll(std::string& out, int32_t expected_len) 
{
    out.clear();
    while ((int32_t)out.size() < expected_len) 
    {
        char buffer[4096];
        int remain = expected_len - out.size();
        int to_read = std::min(remain, (int)sizeof(buffer));
        ssize_t n = ::recv(sock_fd_, buffer, to_read, 0);
        if (n <= 0) return false;
        out.append(buffer, n);
    }
    return true;
}
