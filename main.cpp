#include "NetPack.h"
#include "network/TcpServer.h"
#include "Busd.h"
#include "ModuleManager.h"
#include "ConfigUpdate.cpp"  // 或者 .h + .cpp 分离
#include <csignal>
#include <memory>
#include "Channel.h"
#include "Timer.h"

// 全局通道
Channel<std::pair<int64_t, std::shared_ptr<NetPack>>> server_to_busd;
Channel<std::pair<int64_t, std::shared_ptr<NetPack>>> busd_to_server;
Timer loop;

int main() 
{
    signal(SIGPIPE, SIG_IGN);

    Busd bus(&loop, &server_to_busd, &busd_to_server);
    bus.start();

    ModuleManager manager(bus, &loop);

    manager.registerAllModule();

    TcpServer server(8887, &server_to_busd, &busd_to_server, &loop);

    server.start();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
 
    return 0;
}
