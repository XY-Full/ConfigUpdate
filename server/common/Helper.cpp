#include "Helper.h"
#include "Log.h"
#include <iomanip>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <cstring>
#include <iconv.h>
#endif

#if defined(_WIN32)

// Helper for Windows: Convert encoding via wide char
std::string convert_encoding_win(const std::string &input, UINT fromCP, UINT toCP)
{
    if (input.empty())
        return "";

    int wlen = MultiByteToWideChar(fromCP, 0, input.c_str(), -1, nullptr, 0);
    if (wlen <= 0)
        throw std::runtime_error("MultiByteToWideChar failed");

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(fromCP, 0, input.c_str(), -1, &wstr[0], wlen);

    int olen = WideCharToMultiByte(toCP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (olen <= 0)
        throw std::runtime_error("WideCharToMultiByte failed");

    std::string result(olen, 0);
    WideCharToMultiByte(toCP, 0, wstr.c_str(), -1, &result[0], olen, nullptr, nullptr);

    result.pop_back(); // Remove null terminator
    return result;
}

std::string Helper::utf8_to_local(const std::string &utf8Str)
{
    return convert_encoding_win(utf8Str, CP_UTF8, CP_ACP);
}

std::string Helper::local_to_utf8(const std::string &localStr)
{
    return convert_encoding_win(localStr, CP_ACP, CP_UTF8);
}

std::string Helper::gbk_to_local(const std::string &gbkStr)
{
    return convert_encoding_win(gbkStr, 936, CP_ACP); // 936 = GBK
}

std::string Helper::local_to_gbk(const std::string &localStr)
{
    return convert_encoding_win(localStr, CP_ACP, 936);
}

#else // POSIX (macOS/Linux)

std::string convert_iconv(const std::string &input, const char *from, const char *to)
{
    iconv_t cd = iconv_open(to, from);
    if (cd == (iconv_t)(-1))
    {
        throw std::runtime_error("iconv_open failed");
    }

    size_t inSize = input.size();
    size_t outSize = inSize * 4;
    std::string output(outSize, 0);

    char *inBuf = const_cast<char *>(input.data());
    char *outBuf = &output[0];
    size_t inBytes = inSize;
    size_t outBytes = outSize;

    size_t res = iconv(cd, &inBuf, &inBytes, &outBuf, &outBytes);
    iconv_close(cd);

    if (res == (size_t)-1)
    {
        throw std::runtime_error("iconv conversion failed");
    }

    output.resize(outSize - outBytes);
    return output;
}

std::string utf8_to_local(const std::string &utf8Str)
{
#if defined(__APPLE__)
    return convert_iconv(utf8Str, "UTF-8", "MACROMAN"); // macOS locale encoding fallback
#else
    return convert_iconv(utf8Str, "UTF-8", "GBK"); // Assume GBK as default locale
#endif
}

std::string local_to_utf8(const std::string &localStr)
{
#if defined(__APPLE__)
    return convert_iconv(localStr, "MACROMAN", "UTF-8");
#else
    return convert_iconv(localStr, "GBK", "UTF-8");
#endif
}

std::string gbk_to_local(const std::string &gbkStr)
{
    // On most Linux systems local encoding is GBK already; pass through
    return gbkStr;
}

std::string local_to_gbk(const std::string &localStr)
{
    // Same as above
    return localStr;
}

#endif // _WIN32

void Helper::printStringData(std::string data)
{
    std::cout << "String content as bytes: ";
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)data[i] << " ";
    }
    std::cout << std::dec << std::endl; // 恢复十进制输出
}
