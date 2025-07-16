#pragma once
#include "Channel.h"
#include "EpollWrapper.h"
#include "SocketWrapper.h"
#include <unordered_map>
#include <thread>
#include <atomic>
#include <memory>

class TcpServer 
{
public:
    TcpServer(int port,
              Channel<std::pair<int64_t, std::string>>* in,
              Channel<std::pair<int64_t, std::string>>* out);
    ~TcpServer();

    void start();
    void stop();

private:
    void acceptLoop();
    void outConsumerLoop();
    void cleanupConnection(int fd);

    int server_fd_;
    EpollWrapper epoller_;
    std::unordered_map<int, int64_t> fd_to_conn_;
    std::unordered_map<int64_t, std::shared_ptr<SocketWrapper>> conn_map_;
    std::atomic<int64_t> next_conn_id_{1};

    std::thread accept_thread_;
    std::thread out_thread_;
    std::atomic<bool> running_{false};

    Channel<std::pair<int64_t, std::string>>* server_to_busd;
    Channel<std::pair<int64_t, std::string>>* busd_to_server;
};