
#include "ILogic.h"
#include "HeartHandler.h"
#include "gate.pb.h"

HeartHandler::HeartHandler(Busd* busd) : ILogic(busd) {}

void HeartHandler::registerHandlers() 
{
    busd_->registerHandler(MSGID::CS_HEART_BEAT,
        std::bind(&HeartHandler::onHeart, this, std::placeholders::_1));
}

void HeartHandler::onHeart(const NetPack& pPack) 
{
    PROCESS_NETPACK_BEGIN(Heart);

    response->set_err(ErrorCode::Error_success);
    response->set_timestamp(time(nullptr));

    PROCESS_NETPACK_END();
}