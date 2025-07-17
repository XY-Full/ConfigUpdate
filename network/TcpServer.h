#pragma once
#include "Channel.h"
#include "EpollWrapper.h"
#include "NetPack.h"
#include "SocketWrapper.h"
#include <unordered_map>
#include <thread>
#include <atomic>
#include <memory>
#include "Timer.h"

class TcpServer 
{
public:
    TcpServer(int port,
              Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
              Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out,
              Timer* loop);
    ~TcpServer();

    void start();
    void stop();

private:
    void acceptLoop();
    void outConsumerLoop();
    void cleanupConnection(int fd);
    void checkHeartbeats();

    int server_fd_;
    Timer* loop_;
    EpollWrapper epoller_;
    std::unordered_map<int, int64_t> fd_to_conn_;
    std::unordered_map<int64_t, std::shared_ptr<SocketWrapper>> conn_map_;
    std::atomic<int64_t> next_conn_id_{1};

    std::thread accept_thread_;
    std::thread out_thread_;
    std::atomic<bool> running_{false};

    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* server_to_busd;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* busd_to_server;

    // 心跳相关参数
    std::unordered_map<int64_t, std::chrono::steady_clock::time_point> last_active_time_;
    std::mutex conn_mutex_;
    const int HEARTBEAT_TIMEOUT_SECONDS = 30;
};