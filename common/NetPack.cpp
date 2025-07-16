#include "NetPack.h"
#include <cstring>
#include <google/protobuf/message.h>

std::shared_ptr<NetPack> NetPack::deserialize(const std::string& data) 
{
    auto pack = std::make_shared<NetPack>();
    if (data.size() < sizeof(int32_t) * 3) return pack;

    const char* ptr = data.data();
    memcpy(&pack->len, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    memcpy(&pack->seq, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    memcpy(&pack->msg_id, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);

    pack->msg.assign(ptr, data.size() - sizeof(int32_t) * 3);
    return pack;
}

std::shared_ptr<std::string> NetPack::serialize() const 
{
    auto out = std::make_shared<std::string>();

    int32_t total_len = sizeof(len) + sizeof(seq) + sizeof(msg_id) + msg.size();
    out->resize(total_len);
    char* ptr = out->data();

    memcpy(ptr, &total_len, sizeof(len)); ptr += sizeof(len);
    memcpy(ptr, &seq, sizeof(seq));       ptr += sizeof(seq);
    memcpy(ptr, &msg_id, sizeof(msg_id)); ptr += sizeof(msg_id);
    memcpy(ptr, msg.data(), msg.size());

    return out;
}