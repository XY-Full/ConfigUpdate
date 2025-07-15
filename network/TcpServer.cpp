#include "TcpServer.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>

TcpServer::TcpServer(int port,
                     Channel<std::pair<int64_t, std::string>>* in,
                     Channel<std::pair<int64_t, std::string>>* out)
    : in_(in), out_(out) 
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd_, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd_, 128);
    epoller_.add(server_fd_);
}

TcpServer::~TcpServer() 
{
    stop();
    ::close(server_fd_);
}

void TcpServer::start() 
{
    running_ = true;
    accept_thread_ = std::thread(&TcpServer::acceptLoop, this);
    out_thread_ = std::thread(&TcpServer::outConsumerLoop, this);
}

void TcpServer::stop() 
{
    running_ = false;
    if (accept_thread_.joinable()) accept_thread_.join();
    if (out_thread_.joinable()) out_thread_.join();
}

void TcpServer::acceptLoop() 
{
    while (running_) 
    {
        for (int fd : epoller_.wait(1000)) 
        {
            if (fd == server_fd_) 
            {
                int client_fd = accept(server_fd_, nullptr, nullptr);
                int64_t conn_id = next_conn_id_++;
                epoller_.add(client_fd);
                conn_map_[conn_id] = std::make_shared<SocketWrapper>(client_fd);
                fd_to_conn_[client_fd] = conn_id;
                std::cout << "New connection: " << conn_id << std::endl;
            } 
            else 
            {
                int64_t conn_id = fd_to_conn_[fd];
                std::string len_buf;
                if (!conn_map_[conn_id]->recvAll(len_buf, 4)) continue;
                int32_t len = *reinterpret_cast<const int32_t*>(len_buf.data());

                std::string full_data;
                if (!conn_map_[conn_id]->recvAll(full_data, len)) continue;

                in_->push({conn_id, full_data});
            }
        }
    }
}

void TcpServer::outConsumerLoop() 
{
    while (running_) 
    {
        auto [conn_id, data] = out_->pop();
        auto it = conn_map_.find(conn_id);
        if (it != conn_map_.end()) 
        {
            it->second->sendAll(data);
        }
    }
}
