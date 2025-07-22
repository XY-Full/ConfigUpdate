#pragma once

#include "JsonConfig.h"

class JsonConfigNode
{
public:
    JsonConfigNode() = default; // 默认构造函数
    JsonConfigNode(JsonConfig *config, const std::string &keyPath);
    JsonConfigNode(JsonConfig *config, json *data,
                   const std::string &keyPath = "");

    // 安全访问方法
    std::optional<JsonConfigNode> tryGet(const std::string &key) const;
    JsonConfigNode getOrCreate(const std::string &key);

    JsonConfigNode operator[](const std::string &key);
    JsonConfigNode operator[](const char *key);
    JsonConfigNode operator[](size_t index);

    template <typename T> operator T() const
    {
        return as<T>();
    }

    template <typename T> JsonConfigNode &operator=(const T &value)
    {
        if (!data_)
        {
            throw std::runtime_error("Assignment to invalid node");
        }
        *data_ = value;
        markDirty();
        return *this;
    }

    template <typename T> void append(const T &value)
    {
        ensureArray();
        data_->push_back(value);
        markDirty();
    }

    template <typename T> T value(T defaultValue = T()) const
    {
        if (!exists())
            return defaultValue;
        try
        {
            return data_->get<T>();
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    template <typename T> T as() const
    {
        if (!exists())
        {
            throw std::runtime_error("Node does not exist: " + keyPath_);
        }
        try
        {
            return data_->get<T>();
        }
        catch (const json::exception &e)
        {
            throw std::runtime_error("Conversion error at " + keyPath_ + ": " +
                                     e.what());
        }
    }

    bool exists() const
    {
        return data_ != nullptr && !data_->is_null();
    }
    bool isObject() const
    {
        return exists() && data_->is_object();
    }
    bool isArray() const
    {
        return exists() && data_->is_array();
    }
    bool isString() const
    {
        return exists() && data_->is_string();
    }
    bool isNumber() const
    {
        return exists() &&
               (data_->is_number_integer() || data_->is_number_float());
    }
    bool isBoolean() const
    {
        return exists() && data_->is_boolean();
    }

    json &raw()
    {
        if (!data_)
        {
            throw std::runtime_error("Access to invalid node: " + keyPath_);
        }
        return *data_;
    }

    const json &raw() const
    {
        if (!data_)
        {
            throw std::runtime_error("Access to invalid node: " + keyPath_);
        }
        return *data_;
    }

    const std::string &keyPath() const
    {
        return keyPath_;
    }

    // 新增：安全转换方法
    template <typename T> std::optional<T> tryAs() const
    {
        if (!exists())
            return std::nullopt;
        try
        {
            return data_->get<T>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    // 新增：转换为数组
    std::vector<JsonConfigNode> asArray() const;

    // 新增：转换为对象
    std::map<std::string, JsonConfigNode> asObject() const;

private:
    void ensureArray()
    {
        if (!data_)
        {
            throw std::runtime_error("Cannot append to invalid node: " +
                                     keyPath_);
        }
        if (!data_->is_array())
        {
            *data_ = json::array();
        }
    }

    void markDirty()
    {
        if (config_)
        {
            config_->markDirty();
            config_->notifyChange(keyPath_, *data_);
        }
    }

    JsonConfig *config_ = nullptr;
    json *data_ = nullptr;
    std::string keyPath_;
};