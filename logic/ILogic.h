#pragma once
#include "Busd.h"

// 业务逻辑接口，强制实现注册方法
class ILogic 
{
public:
    virtual ~ILogic() = default;
    virtual void registerHandlers(Busd& bus) = 0;
};