#include "ConfigResolve.h"
#include <fstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;

ConfigResolve::ConfigResolve(const std::string& file_path, bool auto_save)
    : path_(file_path), auto_save_(auto_save) 
{
    load();
}

void ConfigResolve::load() 
{
    std::ifstream in(path_);
    if (in) 
    {
        try 
        {
            in >> config_;
        } 
        catch (...) 
        {
            std::cerr << "Invalid JSON file: " << path_ << ", using empty config.\n";
            config_ = json::object();
        }
    } 
    else 
    {
        config_ = json::object();
    }
}

bool ConfigResolve::save() 
{
    std::ofstream out(path_);
    if (!out) 
    {
        std::cerr << "Failed to save config to: " << path_ << "\n";
        return false;
    }

    out << config_.dump(4);
    return true;
}

std::vector<std::string> ConfigResolve::splitKey(const std::string& key_path) const 
{
    std::vector<std::string> keys;
    size_t pos = 0, dot;

    while ((dot = key_path.find('.', pos)) != std::string::npos) 
    {
        keys.push_back(key_path.substr(pos, dot - pos));
        pos = dot + 1;
    }

    keys.push_back(key_path.substr(pos));
    return keys;
}

void ConfigResolve::ensurePath(const std::string& key_path) 
{
    auto keys = splitKey(key_path);
    json* node = &config_;

    for (const auto& key : keys) 
    {
        if (!node->contains(key)) 
        {
            (*node)[key] = json::object();  // 默认创建空对象
        }
        node = &(*node)[key];
    }
}

nlohmann::json& ConfigResolve::operator[](const std::string& key_path) 
{
    ensurePath(key_path);
    auto keys = splitKey(key_path);
    json* node = &config_;

    for (const auto& key : keys) 
    {
        node = &(*node)[key];
    }

    if (auto_save_) save();
    return *node;
}

bool ConfigResolve::has(const std::string& key_path) const 
{
    auto keys = splitKey(key_path);
    const json* node = &config_;

    for (const auto& key : keys) 
    {
        if (!node->is_object() || !node->contains(key)) return false;
        node = &(*node)[key];
    }

    return true;
}

template<typename T>
void ConfigResolve::set(const std::string& key_path, const T& value) 
{
    ensurePath(key_path);
    auto keys = splitKey(key_path);
    json* node = &config_;

    for (size_t i = 0; i + 1 < keys.size(); ++i)
    {
        node = &(*node)[keys[i]];
    }

    (*node)[keys.back()] = value;
    if (auto_save_) save();
}

// 显式模板实例化
template void ConfigResolve::set<std::string>(const std::string&, const std::string&);
template void ConfigResolve::set<int>(const std::string&, const int&);
template void ConfigResolve::set<double>(const std::string&, const double&);
template void ConfigResolve::set<bool>(const std::string&, const bool&);
template void ConfigResolve::set<json>(const std::string&, const json&);
