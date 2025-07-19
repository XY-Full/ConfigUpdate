#include "TcpClient.h"
#include "Log.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/socket.h>
#endif

TcpClient::TcpClient(const std::string& ip, int port,
                     Channel<std::shared_ptr<NetPack>>* in,
                     Channel<std::shared_ptr<NetPack>>* out,
                     Timer* timer)
    : ip_(ip), port_(port), recv_channel_(in), send_channel_(out), timer_(timer)
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

TcpClient::~TcpClient()
{
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void TcpClient::start()
{
    running_ = true;
    connectToServer();

    recv_thread_ = std::thread(&TcpClient::recvLoop, this);
    send_thread_ = std::thread(&TcpClient::sendLoop, this);

    timer_->runEvery(1.0f, std::bind(&TcpClient::checkHeartbeat, this));
}

void TcpClient::stop()
{
    running_ = false;

    if (recv_thread_.joinable()) recv_thread_.join();
    if (send_thread_.joinable()) send_thread_.join();

    std::lock_guard<std::mutex> lock(conn_mutex_);
    if (sock_ != -1) {
        close(sock_);
        sock_ = -1;
    }
}

void TcpClient::connectToServer()
{
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        ELOG << "Failed to create socket.";
        return;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr);

    if (connect(sock_, (sockaddr*)&addr, sizeof(addr)) != 0) {
        ELOG << "Failed to connect to server " << ip_ << ":" << port_;
        close(sock_);
        sock_ = -1;
        return;
    }

    socket_ = std::make_unique<SocketWrapper>(sock_);
    last_active_time_ = std::chrono::steady_clock::now();

    ILOG << "Connected to server " << ip_ << ":" << port_;
}

void TcpClient::recvLoop()
{
    while (running_) {
        std::string len_buf;
        if (!socket_->recvAll(len_buf, 4, true)) {
            ELOG << "Receive length failed. Reconnecting...";
            stop();
            start();
            return;
        }

        int32_t len = *reinterpret_cast<const int32_t*>(len_buf.data());
        if (len <= 0 || len > 10 * 1024 * 1024) {
            ELOG << "Invalid message length: " << len;
            continue;
        }

        std::string msg;
        if (!socket_->recvAll(msg, len)) {
            ELOG << "Receive message body failed. Reconnecting...";
            stop();
            start();
            return;
        }

        auto netpack = std::make_shared<NetPack>();
        netpack->deserialize(0, msg);
        recv_channel_->push(netpack);
        last_active_time_ = std::chrono::steady_clock::now();
    }
}

void TcpClient::sendLoop()
{
    while (running_) {
        auto msg = send_channel_->pop();
        socket_->sendAll(*msg->serialize());
    }
}

void TcpClient::checkHeartbeat()
{
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_active_time_).count() > HEARTBEAT_TIMEOUT_SECONDS) {
        ELOG << "Heartbeat timeout. Reconnecting...";
        stop();
        start();
    }
}
