#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include "NetPack.h"
#include "config_update.pb.h"
#include "msg_id.pb.h"
#include <iomanip>


void print_bytes(const std::string& str) {
    std::cout << "String content as bytes: ";
    for (size_t i = 0; i < str.size(); ++i) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)str[i] << " ";
    }
    std::cout << std::dec << std::endl;  // 恢复十进制输出
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket error");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8887);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        close(sock);
        return 1;
    }

    cs::ConfigUpdate config_update;
    config_update.mutable_request()->add_update_file("321");

    NetPack pack(&config_update);
    pack.seq = 1;
    pack.flag = C2S_FLAG;
    pack.msg_id = MSGID::CS_CONFIG_UPDATE;
    pack.uid = 0; // 第一次不用设置，后续服务端响应后再设置

    auto buffer = pack.serialize();

    // 发送数据包
    ssize_t sent = send(sock, buffer->data(), buffer->size(), 0);
    if (sent < 0) {
        perror("send error");
    } else {
        std::cout << "Sent " << sent << " bytes" << std::endl;
    }

    // 可选读取回应
    char recv_buf[1024] = {0};
    ssize_t recvd = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (recvd > 0) 
    {
        recv_buf[recvd] = '\0';
        std::cout << "Received: " << recv_buf << std::endl;
    }

    // 第一个参数暂时无用
    NetPack pack_resp;
    pack_resp.deserialize(0, std::string(recv_buf, recvd));
    cs::ConfigUpdate response;
    response.ParseFromString(pack_resp.msg);
    std::cout << "Response: " << response.response().err() << std::endl;

    close(sock);
    return 0;
}
