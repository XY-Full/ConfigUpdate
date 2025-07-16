#include "NetPack.h"
#include <cstring>
#include <stdexcept>
#include <google/protobuf/message.h>

NetPack NetPack::deserialize(const std::string& data) 
{
    std::cout << "start to deserialize!" << std::endl;
    if (data.size() < 8) 
    {
        throw std::runtime_error("data too short");
    }

    NetPack pack;
    std::memcpy(&pack.seq, data.data(), 4);
    std::memcpy(&pack.msg_id, data.data() + 4, 4);

    std::string payload = data.substr(8);
    std::cout << "deserialize end" << std::endl;

    // auto* dummy_msg = new google::protobuf::Message();
    // pack.msg.reset(dummy_msg);

    return pack;
}

std::string NetPack::serialize() const 
{
    if (!msg) 
    {
        throw std::runtime_error("NetPack::serialize: msg is nullptr");
    }

    std::string pb_data;
    if (!msg->SerializeToString(&pb_data)) 
    {
        throw std::runtime_error("NetPack::serialize: failed to serialize protobuf message");
    }

    int32_t body_len = sizeof(seq) + sizeof(msg_id) + pb_data.size();

    std::string buf;
    buf.resize(sizeof(int32_t) + body_len);  // 总长度字段 + body

    char* p = &buf[0];

    std::memcpy(p, &body_len, 4);      // 写入总长度
    std::memcpy(p + 4, &seq, 4);       // 写入 seq
    std::memcpy(p + 8, &msg_id, 4);    // 写入 msg_id
    std::memcpy(p + 12, pb_data.data(), pb_data.size());  // 写入 pb 内容

    return buf;
}
