#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include "google/protobuf/message.h"

enum ServerMsgType : int8_t {
    None 					= 0,
    C2S_FLAG 				= 1,  // 客户端发给服务器
    S2C_FLAG 				= 2,  // 服务器发给客户端
    WEB_REQ_FLAG 			= 3,
    WEB_REP_FLAG 			= 4,
    BROADCAST_FLAG 			= 5,  // 广播
    NOTIFY_FLAG 			= 6,  // 通知某个服务
    S2S_REQ_FLAG 			= 7,  // 请求某个服务
    S2S_REP_FLAG 			= 8,  // 回复某个服务
    NOTIFY_GROUP_FLAG 		= 9,  // 通知某组服务
    GROUPCAST_FLAG 			= 10, // 根据stype组播
    GROUPCAST2_FLAG 		= 11, // 根据sid组播
    NOTIFY_TOTAL_FLAG 		= 12, // 通知所有服务
    PUB_FLAG 				= 13, // 向订阅某个消息的服务发布
    RETRANS_FLAG 			= 14, // 转发
    USR_REQ_FLAG 			= 15, // 请求某个玩家
    USR_REP_FLAG			= 16, // 回复某个玩家	
};

class NetPack : std::enable_shared_from_this<NetPack>
{
public:
    int32_t len     = 0;
    int32_t seq     = 0;
    int32_t msg_id  = 0;
    int64_t conn_id = 0;
    int64_t uid     = 0;
    int8_t  flag    = 0;

    std::string msg;

    NetPack() = default;
    NetPack(const NetPack& request, const google::protobuf::Message* msg, int8_t msg_id = None);
    NetPack(const google::protobuf::Message* msg, int8_t msg_id = None);

    std::shared_ptr<std::string> serialize() const;
    void deserialize(int64_t conn_id, const std::string& data);
};
