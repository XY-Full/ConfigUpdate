#include "Busd.h"
#include <chrono>
#include <iostream>
#include "msg_id.pb.h"

using namespace std::placeholders;

Busd::Busd(Timer* loop,
           Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
           Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out)
    : loop_(loop), in_channel_(in), out_channel_(out), running_(false)
{
    forwardHandleArray_[C2S_FLAG] = std::bind(&Busd::forwardC2S, this, _1);
    forwardHandleArray_[S2C_FLAG] = std::bind(&Busd::forwardS2C, this, _1);
}

void Busd::registerHandler(int32_t msg_id, Handler handler) 
{
    ILOG << "registerHandler by msg_id: " << msg_id;
    handlers_[msg_id] = std::move(handler);
}

void Busd::start() 
{
    running_ = true;
    worker_ = std::thread(&Busd::processMessages, this);
}

void Busd::stop() 
{
    running_ = false;
    if (worker_.joinable()) worker_.join();
}

void Busd::sendToClient(int64_t uid, int32_t msg_id, const google::protobuf::Message& msg) 
{
    auto sendPack = std::make_shared<NetPack>(&msg, S2C_FLAG);
    sendPack->uid = uid;
    sendPack->msg_id = msg_id;

    ILOG << "sendToClient by uid: " << uid << " msg_id: " << MSGID::MsgID_Name(msg_id) << " msg: " << msg.DebugString();

    auto userdata = UsrSvrMap[uid];
    sendPack->conn_id = userdata.connid();
    out_channel_->push({userdata.connid(), sendPack});
}

void Busd::replyToClient(const NetPack& request, const google::protobuf::Message& msg)
{
    auto responsePack = std::make_shared<NetPack>(request, &msg, S2C_FLAG);
    out_channel_->push({request.conn_id, responsePack});
}

void Busd::processMessages() 
{
    while (running_) 
    {
        std::pair<int64_t, std::shared_ptr<NetPack>> item;
        if (!in_channel_) 
        {
            ELOG << "in_channel is nullptr!!";
            return;
        }
        if (in_channel_->try_pop(item)) 
        {
            auto pack = item.second;
            auto forward_func = forwardHandleArray_.at(pack->flag);
            if (!forward_func) 
            {
                ELOG << "invalid msg_type: " << pack->flag;
                continue;
            }
            forward_func(*pack);
        } 
        else 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void Busd::forwardC2S(const NetPack& pack)
{
    int64_t connid = pack.conn_id;
    int32_t msgid = pack.msg_id;
    if (connid == 0) 
    {
        ELOG << "C2S: invalid connid, msgid:" << msgid;
        return;
    }

    int64_t uid = pack.uid;
    auto& userdata = UsrSvrMap[uid];
    if (userdata.connid() != connid) 
    {
        userdata.set_connid(connid);
        userdata.set_ver(userdata.ver() + 1);
    }

    try 
    {
        std::cout << "msg_id : " << pack.msg_id << std::endl;
        auto it = handlers_.find(pack.msg_id);
        if (it != handlers_.end()) 
        {
            ILOG << "trigger handlers!";
            it->second(pack);
        } 
        else 
        {
            ELOG << "msg_id :" << pack.msg_id << " never be registed!";
        }
    } 
    catch (const std::exception& e) 
    {
        ELOG << "deserialize error to NetPack!";
    }
}

void Busd::forwardS2C(const NetPack& pack)
{
}
