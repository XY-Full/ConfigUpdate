#include "JsonConfig.h"
#include "JsonConfigNode.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include "Log.h"

using namespace nlohmann;

JsonConfig::JsonConfig(const std::string &path, LoadMode mode, bool autoSave) : autoSave_(autoSave)
{
    if (!path.empty())
    {
        load(path, mode);
    }
    else
    {
        // 创建空配置对象
        configData_ = json::object();
    }
}

// 移动构造函数
JsonConfig::JsonConfig(JsonConfig &&other) noexcept
{
    // 锁定源对象，确保转移过程中不会被修改
    std::lock_guard<std::mutex> lock(other.callbackMutex_);

    // 转移资源
    configData_ = std::move(other.configData_);
    configPath_ = std::move(other.configPath_);
    loadMode_ = other.loadMode_;
    autoSave_ = other.autoSave_;
    dirty_ = other.dirty_;
    fileConfigs_ = std::move(other.fileConfigs_);

    // 清空源对象状态
    other.configData_ = json{};
    other.configPath_.clear();
    other.loadMode_ = LoadMode::SingleFile;
    other.autoSave_ = false;
    other.dirty_ = false;
    other.fileConfigs_.clear();

    // 注意：不转移回调函数和锁，新对象需要重新注册回调
}

// 移动赋值运算符
JsonConfig &JsonConfig::operator=(JsonConfig &&other) noexcept
{
    if (this != &other)
    {
        // 锁定双方对象，避免死锁
        std::unique_lock<std::mutex> lockThis(callbackMutex_, std::defer_lock);
        std::unique_lock<std::mutex> lockOther(other.callbackMutex_, std::defer_lock);
        std::lock(lockThis, lockOther);

        // 清空当前对象状态
        configData_ = json{};
        fileConfigs_.clear();

        // 转移资源
        configData_ = std::move(other.configData_);
        configPath_ = std::move(other.configPath_);
        loadMode_ = other.loadMode_;
        autoSave_ = other.autoSave_;
        dirty_ = other.dirty_;
        fileConfigs_ = std::move(other.fileConfigs_);

        // 清空源对象状态
        other.configData_ = json{};
        other.configPath_.clear();
        other.loadMode_ = LoadMode::SingleFile;
        other.autoSave_ = false;
        other.dirty_ = false;
        other.fileConfigs_.clear();

        // 注意：回调函数列表不转移，需要在新对象重新注册
    }
    return *this;
}

JsonConfig::~JsonConfig()
{
    if (autoSave_ && dirty_)
    {
        try
        {
            save();
        }
        catch (...)
        {
            // 忽略保存异常
        }
    }
}

bool JsonConfig::load(const std::string &path, LoadMode mode)
{
    configPath_ = fs::path(path);
    loadMode_ = mode;
    fileConfigs_.clear();

    if (configPath_.empty())
    {
        configData_ = json::object();
        return true;
    }

    try
    {
        switch (mode)
        {
        case LoadMode::SingleFile:
            return loadFile(configPath_);
        case LoadMode::Directory:
            return loadDirectory(configPath_, false);
        case LoadMode::MergeDirectory:
            return loadDirectory(configPath_, true);
        }
    }
    catch (const std::exception &e)
    {
        ELOG << "Failed to load config: " << e.what();
        // 加载失败时创建空配置
        configData_ = json::object();
        dirty_ = true;
    }

    return false;
}

bool JsonConfig::loadFile(const fs::path &filePath)
{
    if (!fs::exists(filePath))
    {
        configData_ = json::object();
        dirty_ = true;
        return true;
    }

    try
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open config file: " + filePath.string());
        }

        file >> configData_;
        fileConfigs_[filePath] = configData_;
        dirty_ = false;
        return true;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }

    // 加载失败时创建空配置
    configData_ = json::object();
    dirty_ = true;
    return false;
}

bool JsonConfig::loadDirectory(const fs::path &dirPath, bool merge)
{
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
    {
        configData_ = json::object();
        dirty_ = true;
        return true;
    }

    configData_ = merge ? json::object() : json::array();
    bool success = true;

    try
    {
        for (const auto &entry : fs::directory_iterator(dirPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                try
                {
                    std::ifstream file(entry.path());
                    if (!file.is_open())
                    {
                        ELOG << "Cannot open config file: " << entry.path();
                        success = false;
                        continue;
                    }

                    json fileData;
                    file >> fileData;
                    fileConfigs_[entry.path()] = fileData;

                    if (merge)
                    {
                        configData_.merge_patch(fileData);
                    }
                    else
                    {
                        configData_.push_back({{"file", entry.path().filename().string()}, {"data", fileData}});
                    }
                }
                catch (const json::parse_error &e)
                {
                    ELOG << "JSON parse error in " << entry.path() << ": " << e.what();
                    success = false;
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        ELOG << "Filesystem error: " << e.what();
        success = false;
    }

    dirty_ = false;
    return success;
}

bool JsonConfig::save(const std::string &specificFile)
{
    if (!dirty_ && specificFile.empty())
    {
        return true;
    }

    try
    {
        if (!specificFile.empty())
        {
            return saveToFile(specificFile);
        }

        switch (loadMode_)
        {
        case LoadMode::SingleFile:
            return saveToFile(configPath_);
        case LoadMode::Directory:
        case LoadMode::MergeDirectory:
            bool allSaved = true;
            for (auto &[filePath, fileData] : fileConfigs_)
            {
                if (fileData != configData_)
                {
                    if (!saveToFile(filePath))
                    {
                        allSaved = false;
                    }
                }
            }
            dirty_ = !allSaved;
            return allSaved;
        }
    }
    catch (const std::exception &e)
    {
        ELOG << "Failed to save config: " << e.what();
    }

    return false;
}

bool JsonConfig::saveToFile(const fs::path &filePath)
{
    try
    {
        fs::create_directories(filePath.parent_path());

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open file for writing: " + filePath.string());
        }

        file << configData_.dump(4);
        dirty_ = false;
        return true;
    }
    catch (const std::exception &e)
    {
        ELOG << "Error saving config to " << filePath << ": " << e.what();
        return false;
    }
}

void JsonConfig::addChangeCallback(ChangeCallback callback)
{
    std::lock_guard lock(callbackMutex_);
    changeCallbacks_.push_back(std::move(callback));
}

void JsonConfig::notifyChange(const std::string &key, const json &value)
{
    std::lock_guard lock(callbackMutex_);
    for (auto &callback : changeCallbacks_)
    {
        try
        {
            callback(key, value);
        }
        catch (...)
        {
            ELOG << "Exception in change callback for key: " << key;
            return;
        }
    }
}

void JsonConfig::createDefaultConfig(const std::string &path, const json &defaultConfig)
{
    try
    {
        fs::path filePath(path);
        if (fs::exists(filePath))
            return;

        fs::create_directories(filePath.parent_path());

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot create default config file");
        }

        file << defaultConfig.dump(4);
    }
    catch (const std::exception &e)
    {
        ELOG << "Failed to create default config: " << e.what();
    }
}

std::optional<JsonConfigNode> JsonConfig::getNode(const std::string &path) const
{
    try
    {
        json *current = const_cast<json *>(&configData_);
        std::istringstream iss(path);
        std::string token;

        while (std::getline(iss, token, '.'))
        {
            if (current->is_object() && current->contains(token))
            {
                current = &(*current)[token];
            }
            else
            {
                return std::nullopt;
            }
        }

        return JsonConfigNode(const_cast<JsonConfig *>(this), current, path);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

JsonConfigNode JsonConfig::operator[](const std::string &key)
{
    return JsonConfigNode(this, key);
}

JsonConfigNode JsonConfig::operator[](const char *key)
{
    return JsonConfigNode(this, key);
}
