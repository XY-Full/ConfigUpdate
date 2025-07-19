#include "ModuleManager.h"
#include "HeartHandler.h"
#include "ConfigUpdate.h"

void ModuleManager::registerAllModule() 
{
    heartMgr_ = std::make_shared<HeartHandler>(busd_);
    heartMgr_->registerHandlers();

    configUpdateMgr_ = std::make_shared<ConfigUpdate>(busd_);
    configUpdateMgr_->registerHandlers();
}