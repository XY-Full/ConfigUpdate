#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include "google/protobuf/message.h"

struct NetPack 
{
    int32_t len = 0;
    int32_t seq = 0;
    int32_t msg_id = 0;
    int64_t conn_id = 0;
    std::string msg;

    NetPack() = default;
    NetPack(const NetPack& request);
    NetPack(const NetPack& request, const google::protobuf::Message* msg);
    NetPack& operator=(const NetPack& request);

    std::shared_ptr<std::string> serialize() const;
    static std::shared_ptr<NetPack> deserialize(int64_t conn_id, const std::string& data);
};
