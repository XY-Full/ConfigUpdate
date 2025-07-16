#pragma once
#include <unistd.h>
#include <string>

class SocketWrapper 
{
public:
    explicit SocketWrapper(int fd = -1);
    ~SocketWrapper();

    int fd() const;
    void close();

    bool sendAll(const std::string& data);
    bool recvAll(std::string& out, size_t expected_len);

private:
    int sock_fd_;
};
