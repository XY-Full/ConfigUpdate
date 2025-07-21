#include "SocketWrapper.h"
#include <cstring>
#include "Log.h"
#include <algorithm>

SocketWrapper::SocketWrapper(socket_t fd) : sock_fd_(fd) {}

SocketWrapper::~SocketWrapper() 
{
    close();
}

socket_t SocketWrapper::fd() const 
{
    return sock_fd_;
}

void SocketWrapper::close() 
{
    if (sock_fd_ != -1) {
#ifdef _WIN32
        closesocket(sock_fd_);
#else
        ::close(sock_fd_);
#endif
        sock_fd_ = -1;
    }
}

bool SocketWrapper::sendAll(const std::string& data) 
{
    size_t total_sent = 0;
    while (total_sent < data.size()) 
    {
        int sent = ::send(sock_fd_, data.data() + total_sent,
                          static_cast<int>(data.size() - total_sent), 0);
        if (sent <= 0) 
        {
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool SocketWrapper::recvAll(std::string& out, size_t size, bool use_peek) 
{
    out.clear();
    if (sock_fd_ < 0) 
    {
        ELOG << "Invalid socket";
        return false;
    }

    while (out.size() < size) 
    {
        char buf[1024];
        size_t to_read = std::min(sizeof(buf), size - out.size());

        int flags = use_peek ? MSG_PEEK : 0;
        int n = ::recv(sock_fd_, buf, static_cast<int>(to_read), flags);

        if (n < 0) 
        {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return false;
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
            perror("recv error");
#endif
            return false;
        } 
        else if (n == 0) 
        {
            ELOG << "Remote side closed connection.\n";
            return false;
        }

        if (!use_peek)
        {
            out.append(buf, n);
        }
        else 
        {
            // 如果使用peek模式，直接原地构造，防止死循环
            out.assign(buf, n);
            break;
        }
    }
    return true;
}
