
#include "ConfigUpdate.h"
#include "ILogic.h"
#include "config_update.pb.h"
#include "msg_id.pb.h"
#include "notify.pb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

ConfigUpdate::ConfigUpdate(Busd* busd) : ILogic(busd) {}

void ConfigUpdate::registerHandlers() 
{
    busd_->registerHandler(MSGID::CS_CONFIG_UPDATE,
        std::bind(&ConfigUpdate::onConfigRequest, this, std::placeholders::_1));
}

void ConfigUpdate::onConfigRequest(const NetPack& pPack) 
{
    PROCESS_NETPACK_BEGIN(cs::ConfigUpdate);

    int pipe_fd[2];
    pipe(pipe_fd);

    std::string param;
    ILOG << request->update_file_size();
    for(const auto& it : request->update_file())
    {
        param += it + " ";
    }
    ILOG << "param: " << param;

    pid_t pid = fork();
    // 子进程
    if (pid == 0) 
    { 
        close(pipe_fd[0]); // 关闭读端
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        std::string cmd = "/usr/bin/config_update.sh " + param;
        execlp("bash", "bash", "-c", cmd.c_str(), (char*)nullptr);
        perror("execl failed");
        exit(1);
    } 
    // 父进程
    else if (pid > 0) 
    {
        close(pipe_fd[1]); // 关闭写端
        char output[BUFFER_SIZE];
        ssize_t read_bytes;
        
        while ((read_bytes = read(pipe_fd[0], output, sizeof(output))) > 0) 
        {
            memset(output, 0, sizeof(output));

            read_bytes = read(pipe_fd[0], output, sizeof(output));

            cs::Notify notify;
            notify.mutable_config_update_output()->set_context(output, read_bytes);
            busd_->sendToClient(uid, MSGID::SC_NOTIFY, notify);
        }

        close(pipe_fd[0]);
        waitpid(pid, nullptr, 0); // 等待子进程结束
    }

    response->set_err(ErrorCode::Error_success);

    PROCESS_NETPACK_END();
}