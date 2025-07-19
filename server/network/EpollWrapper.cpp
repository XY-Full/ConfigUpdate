#include "EpollWrapper.h"
#include <sys/epoll.h>
#include <unistd.h>

EpollWrapper::EpollWrapper() 
{
    epoll_fd_ = epoll_create1(0);
}

EpollWrapper::~EpollWrapper() 
{
    if (epoll_fd_ != -1) ::close(epoll_fd_);
}

bool EpollWrapper::add(int fd) 
{
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool EpollWrapper::remove(int fd) 
{
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

std::vector<int> EpollWrapper::wait(int timeout_ms) 
{
    epoll_event events[64];
    int nfds = epoll_wait(epoll_fd_, events, 64, timeout_ms);
    std::vector<int> result;
    for (int i = 0; i < nfds; ++i) 
    {
        result.push_back(events[i].data.fd);
    }
    return result;
}
