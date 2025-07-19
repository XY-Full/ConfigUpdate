#include "TcpServer.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include "Log.h"

TcpServer::TcpServer(int port,
                     Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out,
                     Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
                     Timer* loop)
    : server_to_busd(out), busd_to_server(in), loop_(loop)
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        ELOG << "Failed to bind to port " << port;
        exit(1);
    }

    ILOG << "Server started on port " << port;

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
    loop_->runEvery(1.0f, std::bind(&TcpServer::checkHeartbeats, this));
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
                if (!conn_map_[conn_id]->recvAll(len_buf, 4, true)) 
                {
                    std::cout << "Failed to receive length from conn " << conn_id << ", closing connection\n";
                    cleanupConnection(fd);
                    continue;
                }

                // 防止非数字导致长度超大
                int32_t len = *reinterpret_cast<const int32_t*>(len_buf.data());
                if (len <= 0 || len > 10 * 1024 * 1024) 
                {
                    std::cout << "Invalid message length: " << len << ", closing connection " << conn_id << std::endl;
                    cleanupConnection(fd);
                    continue;
                }

                std::string full_data;
                if (!conn_map_[conn_id]->recvAll(full_data, len)) 
                {
                    std::cout << "Failed to receive full message from conn " << conn_id << ", closing connection\n";
                    cleanupConnection(fd);
                    continue;
                }

                ILOG << "recv from [" << conn_id << "] : " << full_data;
                auto recv_pack = std::make_shared<NetPack>();
                recv_pack->deserialize(conn_id, full_data);
                server_to_busd->push({conn_id, recv_pack});
                last_active_time_[conn_id] = std::chrono::steady_clock::now();
            }
        }
    }
}

void TcpServer::checkHeartbeats()
{
    std::lock_guard<std::mutex> lock(conn_mutex_);
    auto now = std::chrono::steady_clock::now();
    std::vector<int> fds_to_close;

    for (const auto& [conn_id, last_time] : last_active_time_) 
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count() > HEARTBEAT_TIMEOUT_SECONDS) 
        {
            std::cout << "Heartbeat timeout for conn " << conn_id << ", kicking..." << std::endl;
            for (const auto& [fd, id] : fd_to_conn_) 
            {
                if (id == conn_id) 
                {
                    fds_to_close.push_back(fd);
                    break;
                }
            }
        }
    }

    for (int fd : fds_to_close) 
    {
        cleanupConnection(fd);
    }
}

void TcpServer::cleanupConnection(int fd) 
{
    auto it = fd_to_conn_.find(fd);
    if (it != fd_to_conn_.end()) 
    {
        int64_t conn_id = it->second;
        conn_map_.erase(conn_id);
        fd_to_conn_.erase(fd);
        last_active_time_.erase(it->second);
        epoller_.remove(fd);
        ::close(fd);
        std::cout << "Closed connection: " << conn_id << std::endl;
    }
}

void TcpServer::outConsumerLoop() 
{
    while (running_) 
    {
        auto [conn_id, data] = busd_to_server->pop();
        auto it = conn_map_.find(conn_id);
        if (it != conn_map_.end()) 
        {
            it->second->sendAll(*data->serialize());
        }
    }
}
