#include "ILogic.h"

class HeartHandler : public ILogic 
{
public:
    HeartHandler(Busd*);
    
    void registerHandlers() override;

private:
    void onHeart(const NetPack& msg);
};