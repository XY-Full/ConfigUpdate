#include "NetPack.h"
#include "network/TcpServer.h"
#include "Busd.h"
#include "ModuleManager.h"
#include "ConfigUpdate.cpp"
#include "Channel.h"
#include "Timer.h"
#include "JsonConfig.h"
#include "JsonConfigNode.h"

#include <csignal>
#include <memory>

// 全局通道
Channel<std::pair<int64_t, std::shared_ptr<NetPack>>> server_to_busd;
Channel<std::pair<int64_t, std::shared_ptr<NetPack>>> busd_to_server;
Timer loop;
JsonConfig config_resolver("config.json", JsonConfig::LoadMode::SingleFile, true);

int main() 
{
    signal(SIGPIPE, SIG_IGN);

    int32_t port = config_resolver["server"]["port"].value(8888);

    Busd bus(&loop, &server_to_busd, &busd_to_server);
    bus.start();

    ModuleManager manager(&bus, &loop);

    manager.registerAllModule();

    TcpServer server(port, &server_to_busd, &busd_to_server, &loop);

    server.start();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
 
    return 0;
}
