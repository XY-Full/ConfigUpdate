#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include "base.pb.h"

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

    int32_t seq = 1;
    int32_t msg_id = 1;
    ConfigUpdate config_update;
    config_update.add_update_file("321");
    std::string pb_data;
    config_update.SerializeToString(&pb_data);

    // 构造整个消息体（长度 = seq + msg_id + pb_data）
    int32_t len = sizeof(seq) + sizeof(msg_id) + pb_data.size();

    std::string buffer;
    buffer.resize(4 + len); // 总大小 = length字段(4) + 实际数据长度

    // 填写 length 字段（前4字节）
    std::memcpy(&buffer[0], &len, 4);
    // 填写 seq（第5~8字节）
    std::memcpy(&buffer[4], &seq, 4);
    // 填写 msg_id（第9~12字节）
    std::memcpy(&buffer[8], &msg_id, 4);
    // 填写 pb_data（第13字节开始）
    std::memcpy(&buffer[12], pb_data.data(), pb_data.size());

    // 发送数据包
    ssize_t sent = send(sock, buffer.data(), buffer.size(), 0);
    if (sent < 0) {
        perror("send error");
    } else {
        std::cout << "Sent " << sent << " bytes" << std::endl;
    }

    // 可选读取回应
    char recv_buf[1024] = {0};
    ssize_t recvd = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (recvd > 0) {
        recv_buf[recvd] = '\0';
        std::cout << "Received: " << recv_buf << std::endl;
    }

    close(sock);
    return 0;
}
