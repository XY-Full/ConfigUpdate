#include "JsonConfigNode.h"

JsonConfigNode::JsonConfigNode(JsonConfig *config, const std::string &keyPath)
    : config_(config), keyPath_(keyPath)
{

    if (config_ && !keyPath.empty())
    {
        json *current = &config_->root();
        std::istringstream iss(keyPath);
        std::string token;
        std::string currentPath;

        while (std::getline(iss, token, '.'))
        {
            if (currentPath.empty())
            {
                currentPath = token;
            }
            else
            {
                currentPath += "." + token;
            }

            if (current->is_object())
            {
                if (!current->contains(token))
                {
                    (*current)[token] = json::object();
                }
                current = &(*current)[token];
            }
            else if (current->is_array())
            {
                try
                {
                    size_t index = std::stoul(token);
                    if (index < current->size())
                    {
                        current = &(*current)[index];
                    }
                    else
                    {
                        data_ = nullptr;
                        return;
                    }
                }
                catch (...)
                {
                    data_ = nullptr;
                    return;
                }
            }
            else
            {
                data_ = nullptr;
                return;
            }
        }

        data_ = current;
    }
}

JsonConfigNode::JsonConfigNode(JsonConfig *config, json *data,
                               const std::string &keyPath)
    : config_(config), data_(data), keyPath_(keyPath)
{
}

std::optional<JsonConfigNode> JsonConfigNode::tryGet(
    const std::string &key) const
{
    if (!data_ || !data_->is_object())
    {
        return std::nullopt;
    }

    if (!data_->contains(key))
    {
        return std::nullopt;
    }

    std::string newPath = keyPath_.empty() ? key : keyPath_ + "." + key;
    return JsonConfigNode(config_, &(*data_)[key], newPath);
}

JsonConfigNode JsonConfigNode::getOrCreate(const std::string &key)
{
    if (!data_)
    {
        throw std::runtime_error("Cannot create child on invalid node: " +
                                 keyPath_);
    }

    if (!data_->is_object())
    {
        throw std::runtime_error("Parent node is not an object: " + keyPath_);
    }

    if (!data_->contains(key))
    {
        (*data_)[key] = json::object();
    }

    std::string newPath = keyPath_.empty() ? key : keyPath_ + "." + key;
    return JsonConfigNode(config_, &(*data_)[key], newPath);
}

JsonConfigNode JsonConfigNode::operator[](const std::string &key)
{
    if (!data_)
    {
        throw std::runtime_error("Cannot access child of invalid node: " + keyPath_);
    }

    if (data_->is_object())
    {
        std::string newPath = keyPath_.empty() ? key : keyPath_ + "." + key;
        return JsonConfigNode(config_, &(*data_)[key], newPath);
    }
    else if (data_->is_array())
    {
        try
        {
            size_t index = std::stoul(key);
            if (index < data_->size())
            {
                std::string newPath = keyPath_ + "[" + key + "]";
                return JsonConfigNode(config_, &(*data_)[index], newPath);
            }
            throw std::out_of_range("Array index out of bounds: " + keyPath_);
        }
        catch (const std::invalid_argument &)
        {
            throw std::runtime_error("Invalid array index: " + keyPath_);
        }
    }

    throw std::runtime_error("Cannot access child of non-container node: " + keyPath_);
}

JsonConfigNode JsonConfigNode::operator[](const char *key)
{
    return operator[](std::string(key));
}

JsonConfigNode JsonConfigNode::operator[](size_t index)
{
    if (!data_)
    {
        throw std::runtime_error("Cannot access child of invalid node: " + keyPath_);
    }

    if (!data_->is_array())
    {
        throw std::runtime_error("Node is not an array: " + keyPath_);
    }

    if (index >= data_->size())
    {
        throw std::out_of_range("Array index out of bounds: " + keyPath_);
    }

    std::string newPath = keyPath_ + "[" + std::to_string(index) + "]";
    return JsonConfigNode(config_, &(*data_)[index], newPath);
}

// 将JsonConfigNode对象转换为JsonConfigNode数组
std::vector<JsonConfigNode> JsonConfigNode::asArray() const
{
    std::vector<JsonConfigNode> result;

    // 如果data_存在且为数组类型
    if (data_ && data_->is_array())
    {
        for (size_t i = 0; i < data_->size(); ++i)
        {
            // 构造新的路径，形如 [1]
            std::string newPath = keyPath_ + "[" + std::to_string(i) + "]";
            // 将新的JsonConfigNode对象添加到数组中
            result.emplace_back(config_, &(*data_)[i], newPath);
        }
    }

    return result;
}

// 将JsonConfigNode转换为std::map<std::string, JsonConfigNode>类型
std::map<std::string, JsonConfigNode> JsonConfigNode::asObject() const
{
    std::map<std::string, JsonConfigNode> result;

    // 如果data_存在且为json对象类型
    if (data_ && data_->is_object())
    {
        for (auto &[key, value] : data_->items())
        {
            // 如果keyPath_为空，则newPath为key，否则newPath为keyPath_ + "." + key
            std::string newPath = keyPath_.empty() ? key : keyPath_ + "." + key;
            // 将key和当前节点插入到result中
            result.emplace(key, JsonConfigNode(config_, &value, newPath));
        }
    }

    return result;
}