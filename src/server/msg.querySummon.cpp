#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "message.hpp"

namespace server {

class querySummonHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      sst::dict summonInfo;
      summonInfo.add<sst::str>("normal-info") = "10 gems: 70% R, 20% SR, 10% SSR";
      summonInfo.add<sst::str>("ur-info") = "100 gems: 77% SR, 20% SSR, 3% UR";
      auto& maxBuy = summonInfo.add<sst::array>("max-buy");

      auto gems = ctxt.pAcct->dict()["gems"].as<sst::mint>().get();

      maxBuy.append<sst::mint>() = (gems / 10);
      maxBuy.append<sst::mint>() = (gems / 100);

      ch.sendSst(summonInfo);
   }
};

namespace { msgRegistrar<querySummonHandler> registrar("querySummon"); }

} // namespace server
