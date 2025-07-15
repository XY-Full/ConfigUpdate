#include "network/TcpServer.h"
#include "Busd.h"
#include "ModuleManager.h"
#include "ConfigUpdate.cpp"  // 或者 .h + .cpp 分离
#include <csignal>
#include "Channel.h"

// 全局通道
Channel<std::pair<int64_t, std::string>> server_to_busd;
Channel<std::pair<int64_t, std::string>> busd_to_server;

int main() 
{
    signal(SIGPIPE, SIG_IGN);

    Busd bus;
    ModuleManager manager(bus);

    // 动态注册所有模块
    ConfigModule configMod;
    manager.registerModule(&configMod);

    TcpServer server(8888, &server_to_busd, &busd_to_server);
    return 0;
}