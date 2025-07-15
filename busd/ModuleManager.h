#pragma once
#include "Busd.h"
#include <vector>

class ILogic;

class ModuleManager 
{
public:
    explicit ModuleManager(Busd& bus)
        : bus_(bus) {}

    // 注册模块（每个模块自注册消息 -> 处理函数）
    void registerModule(ILogic* module);

private:
    Busd& bus_;
    std::vector<ILogic*> modules_;
};