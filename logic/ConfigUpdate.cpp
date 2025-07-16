
#include "ILogic.h"
#include "base.pb.h"
#include "msg_id.pb.h"
#include <memory>

class ConfigModule : public ILogic {
public:
    void registerHandlers(Busd& bus) override 
    {
        bus.register_handler( 
            MSGID::CS_CONFIG_UPDATE,
            std::bind(&ConfigModule::onConfigRequest, this, std::placeholders::_1, std::placeholders::_2)
        );

        bus.register_handler( 
            1,
            std::bind(&ConfigModule::onConfigRequest, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

private:
    void onConfigRequest(int64_t uid, const NetPack& msg) 
    {
        auto request = std::make_shared<ConfigUpdate>();
        request->ParseFromString(msg.msg);
        std::cout << "config_update!!!" << std::endl;
        std::cout << "logic recv : " << msg.msg_id << std::endl;
        std::cout << "msg body : " << request->update_file(0) << std::endl;
    }
};