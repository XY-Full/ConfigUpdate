
#include "ILogic.h"

class ConfigUpdate : public ILogic 
{
public:
    ConfigUpdate(Busd* busd);

    void registerHandlers() override;

private:
    void onConfigRequest(const NetPack& pPack);
};