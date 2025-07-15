#include "ModuleManager.h"
#include "ILogic.h"

void ModuleManager::registerModule(ILogic* module) 
{
    module->registerHandlers(bus_);
    modules_.push_back(module);
}