#include "ILogic.h"

class HeartHandler : public ILogic {
public:
    void registerHandlers() override;

private:
    void onHeart(const NetPack& msg);
};