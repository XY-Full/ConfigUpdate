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
    std::string msg;

    std::string serialize() const;
    static NetPack deserialize(const std::string& data);
};
