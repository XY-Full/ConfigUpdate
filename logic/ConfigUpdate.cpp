
#include "ConfigUpdate.h"
#include "config_update.pb.h"
#include "msg_id.pb.h"

void ConfigModule::registerHandlers() 
{
    busd_->registerHandler( 
        MSGID::CS_CONFIG_UPDATE,
        std::bind(&ConfigModule::onConfigRequest, this, std::placeholders::_1)
    );
}

void ConfigModule::onConfigRequest(const NetPack& pPack) 
{
    PROCESS_NETPACK_BEGIN(cs::ConfigUpdate);

    response->set_err(ErrorCode::Error_success);

    PROCESS_NETPACK_END();
}