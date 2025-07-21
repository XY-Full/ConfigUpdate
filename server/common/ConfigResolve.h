#pragma once

#include <string>
#include <vector>
#include "json.hpp"

class ConfigResolve 
{
public:
    ConfigResolve(const std::string& file_path = "config.json", bool auto_save = false);

    // 获取或创建节点
    nlohmann::json& operator[](const std::string& key_path);

    // 判断节点是否存在
    bool has(const std::string& key_path) const;

    // 设置值（支持类型动态转换）
    template<typename T>
    void set(const std::string& key_path, const T& value);

    // 保存到文件
    bool save();

private:
    std::string path_;
    bool auto_save_;
    nlohmann::json config_;

    void load();
    void ensurePath(const std::string& key_path);
    std::vector<std::string> splitKey(const std::string& key_path) const;
};
