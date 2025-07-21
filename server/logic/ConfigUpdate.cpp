
#include "ConfigUpdate.h"
#include "ILogic.h"
#include "config_update.pb.h"
#include "msg_id.pb.h"
#include "notify.pb.h"

ConfigUpdate::ConfigUpdate(Busd* busd) : ILogic(busd) {}

void ConfigUpdate::registerHandlers() 
{
    busd_->registerHandler(MSGID::CS_CONFIG_UPDATE,
        std::bind(&ConfigUpdate::onConfigRequest, this, std::placeholders::_1));
}

void ConfigUpdate::onConfigRequest(const NetPack& pPack) 
{
    PROCESS_NETPACK_BEGIN(cs::ConfigUpdate);

    ILOG << request->update_file(0);

    for(int32_t i; i < 5; i++)
    {
        cs::Notify notify;
        notify.mutable_config_update_output()->set_context("updateing...");
        busd_->sendToClient(uid, MSGID::SC_NOTIFY, notify);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    response->set_err(ErrorCode::Error_success);

    PROCESS_NETPACK_END();
}