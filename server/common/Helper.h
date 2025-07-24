#pragma once

#include "string"
class Helper
{
public:
    static void printStringData(std::string data);
    static std::string utf8_to_local(const std::string &utf8Str);
    static std::string local_to_utf8(const std::string &localStr);
    static std::string gbk_to_local(const std::string &gbkStr);
    static std::string local_to_gbk(const std::string &localStr);
};