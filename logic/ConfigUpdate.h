
#include "ILogic.h"

class ConfigModule : public ILogic {
public:
    void registerHandlers() override;

private:
    void onConfigRequest(const NetPack& pPack);
};