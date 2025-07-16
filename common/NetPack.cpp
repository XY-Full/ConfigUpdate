#include "NetPack.h"
#include <cstring>
#include <stdexcept>
#include <google/protobuf/message.h>

NetPack NetPack::deserialize(const std::string& data) 
{
    if (data.size() < 12) 
    {
        throw std::runtime_error("NetPack::deserialize: data too short");
    }

    NetPack pack;
    std::memcpy(&pack.len, data.data(), 4);
    std::memcpy(&pack.seq, data.data() + 4, 4);
    std::memcpy(&pack.msg_id, data.data() + 8, 4);

    size_t expected_size = 4 + pack.len;
    if (data.size() < expected_size) 
    {
        throw std::runtime_error("NetPack::deserialize: incomplete data");
    }

    pack.msg = data.substr(12, pack.len - 8);  // len includes only seq + msg_id + msg
    return pack;
}

std::string NetPack::serialize() const 
{
    std::string buffer;
    
    // 计算有效负载长度（不包括 len 自身）
    int32_t payload_len = sizeof(seq) + sizeof(msg_id) + msg.size();
    
    buffer.resize(sizeof(len) + payload_len);  // 4 + 4 + 4 + msg.size()

    char* p = &buffer[0];
    std::memcpy(p, &payload_len, sizeof(len));
    std::memcpy(p + 4, &seq, sizeof(seq));
    std::memcpy(p + 8, &msg_id, sizeof(msg_id));
    std::memcpy(p + 12, msg.data(), msg.size());

    return buffer;
}