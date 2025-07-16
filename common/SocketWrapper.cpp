#include "SocketWrapper.h"
#include <sys/socket.h>
#include <cstring>
#include <iostream>

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

bool SocketWrapper::recvAll(std::string& out, size_t size) 
{
    out.clear();
    while (out.size() < size) 
    {
        char buf[1024];
        size_t to_read = std::min(sizeof(buf), size - out.size());
        ssize_t n = recv(sock_fd_, buf, to_read, 0);
        if (n < 0) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                // 非阻塞读取未就绪，直接返回失败
                return false;
            } 
            else 
            {
                perror("recv error");
                return false;
            }
        } 
        else if (n == 0) 
        {
            // 客户端断开连接
            std::cout << "Client disconnected\n";
            return false;
        }
        out.append(buf, n);
    }
    return true;
}
