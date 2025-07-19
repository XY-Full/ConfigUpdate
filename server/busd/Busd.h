#pragma once
#include "Channel.h"
#include "NetPack.h"
#include "Timer.h"
#include "Log.h"
#include "core_user.pb.h"
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>
#include <array>

class Busd 
{
public:
    using Handler = std::function<void(const NetPack&)>;
    using ForwardHandleFunc = std::function<void(const NetPack&)>;

    Busd(Timer* loop,
         Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
         Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out);

    void registerHandler(int32_t msg_id, Handler handler);
    void start();
    void stop();

    void sendToClient(int64_t uid, int32_t msg_id, const google::protobuf::Message& msg);
    void replyToClient(const NetPack& request, const google::protobuf::Message& msg);

private:
    void processMessages();
    void forwardC2S(const NetPack& pack);
    void forwardS2C(const NetPack& pack);

    std::unordered_map<int32_t, Handler> handlers_;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in_channel_ = nullptr;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out_channel_ = nullptr;
    Timer* loop_ = nullptr;
    std::thread worker_;
    std::atomic<bool> running_ = false;

    std::unordered_map<int64_t, core::UsrSvrMappingData> UsrSvrMap;
    std::array<ForwardHandleFunc, std::numeric_limits<int8_t>::max()> forwardHandleArray_;
};
