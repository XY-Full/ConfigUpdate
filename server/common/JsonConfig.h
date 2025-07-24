#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

class JsonConfigNode;

class JsonConfig
{
    friend class JsonConfigNode;

public:
    enum class LoadMode
    {
        SingleFile,
        Directory,
        MergeDirectory
    };

    explicit JsonConfig(const std::string &path = "", LoadMode mode = LoadMode::SingleFile, bool autoSave = false);
    
    // 禁止拷贝
    JsonConfig(const JsonConfig &) = delete;
    JsonConfig &operator=(const JsonConfig &) = delete;

    // 移动构造函数
    JsonConfig(JsonConfig &&other) noexcept;

    // 移动赋值运算符
    JsonConfig &operator=(JsonConfig &&other) noexcept;

    ~JsonConfig();

    std::string getPath() { return configPath_.string(); }

    bool load(const std::string &path = "",
              LoadMode mode = LoadMode::SingleFile);
    bool save(const std::string &specificFile = "");

    JsonConfigNode operator[](const std::string &key);
    JsonConfigNode operator[](const char *key);

    json &root()
    {
        return configData_;
    }
    const json &root() const
    {
        return configData_;
    }

    void setAutoSave(bool enable)
    {
        autoSave_ = enable;
    }
    bool autoSave() const
    {
        return autoSave_;
    }

    void markDirty()
    {
        dirty_ = true;
    }
    bool isDirty() const
    {
        return dirty_;
    }

    using ChangeCallback =
        std::function<void(const std::string &key, const json &value)>;
    void addChangeCallback(ChangeCallback callback);

    static void createDefaultConfig(const std::string &path,
                                    const json &defaultConfig);

    // 安全获取节点
    std::optional<JsonConfigNode> getNode(const std::string &path) const;

private:
    bool loadFile(const fs::path &filePath);
    bool loadDirectory(const fs::path &dirPath, bool merge);
    bool saveToFile(const fs::path &filePath);
    void notifyChange(const std::string &key, const json &value);

    json configData_;
    fs::path configPath_;
    LoadMode loadMode_ = LoadMode::SingleFile;
    bool autoSave_ = false;
    bool dirty_ = false;
    std::map<fs::path, json> fileConfigs_;
    std::vector<ChangeCallback> changeCallbacks_;
    mutable std::mutex callbackMutex_;
};
