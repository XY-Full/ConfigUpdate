#include "NetPack.h"
#include <cstring>
#include <google/protobuf/message.h>

NetPack::NetPack(const NetPack& other)
{
    len = other.len;
    seq = other.seq;
    msg_id = other.msg_id;
    conn_id = other.conn_id;
}

NetPack& NetPack::operator=(const NetPack& other)
{
    if (this != &other)
    {
        len = other.len;
        seq = other.seq;
        msg_id = other.msg_id;
        conn_id = other.conn_id;
    }
    return *this;
}

NetPack::NetPack(const NetPack& other, const google::protobuf::Message* msg)
{
    len = other.len;
    seq = other.seq;
    msg_id = other.msg_id;
    conn_id = other.conn_id;
    if(msg)
        this->msg = msg->SerializeAsString();
}

std::shared_ptr<NetPack> NetPack::deserialize(int64_t conn_id, const std::string& data) 
{
    auto pack = std::make_shared<NetPack>();
    if (data.size() < (sizeof(int32_t) * 3 + sizeof(int64_t))) return pack;

    const char* ptr = data.data();
    memcpy(&pack->len, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    memcpy(&pack->seq, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    memcpy(&pack->msg_id, ptr, sizeof(int32_t)); ptr += sizeof(int32_t);
    memcpy(&pack->conn_id, ptr, sizeof(int64_t)); ptr += sizeof(int64_t);

    pack->msg.assign(ptr, data.size() - sizeof(int32_t) * 3 - sizeof(int64_t));
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
    memcpy(ptr, &conn_id, sizeof(conn_id)); ptr += sizeof(conn_id);
    memcpy(ptr, msg.data(), msg.size());

    return out;
}