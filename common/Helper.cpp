#include "Helper.h"
#include "Log.h"
#include <iomanip>

void Helper::printStringData(std::string data)
{
    std::cout << "String content as bytes: ";
    for (size_t i = 0; i < data.size(); ++i) 
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)data[i] << " ";
    }
    std::cout << std::dec << std::endl;  // 恢复十进制输出
}