#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>
#include <set>

namespace server {

class boostCharHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      std::unique_ptr<sst::dict> pReq(ch.recvSst());
      auto charId = (*pReq)["char"].as<sst::str>().get();
      auto& charOverlay = ctxt.pAcct->dict()
         ["chars"].as<sst::dict>()
            [charId].as<sst::dict>();

      auto mode = (*pReq)["action"].as<sst::str>().get();
      std::string error;
      if(mode == "level-up")
         levelUp(charOverlay,error,ctxt.pAcct->dict());
      else if(mode == "star-up")
         starUp(charOverlay,error,ctxt.pAcct->dict());
      else if(mode == "change-equip")
      {
         auto itemId = (*pReq)["item"].as<sst::mint>().get();
         auto itemType = (*pReq)["item-type"].as<sst::mint>().get();
         changeEquip(charOverlay,error,ctxt.pAcct->dict(),itemId,itemType);
      }
      else
         throw std::runtime_error("ISE");

      ch.sendString(error);
      if(error.empty())
         ch.sendSst(ctxt.pAcct->dict());
   }

private:
   void levelUp(sst::dict& Char, std::string& error, sst::dict& acct)
   {
      auto& l = Char["level"].as<sst::mint>();
      auto& s = Char["stars"].as<sst::mint>();
      auto& g = acct["gold"].as<sst::mint>();

      if(g.get() < 100)
         error = "insufficient funds";
      else if(l.get() >= s.get()*10)
         error = "max level reached";
      else
      {
         // boost it
         l = l.get() + 1;
         g = g.get() - 100;
      }
   }

   void starUp(sst::dict& Char, std::string& error, sst::dict& acct)
   {
      auto& stars = Char["stars"].as<sst::mint>();
      if(stars.get() == 6)
      {
         error = "max stars reached";
         return;
      }

      std::stringstream sKey;
      sKey << Char["type"].as<sst::mint>().get();
      auto& inven = acct["items"].as<sst::dict>();
      if(!inven.has(sKey.str()))
      {
         error = "insufficient funds";
         return;
      }

      stars = stars.get() + 1;
      auto& funds = inven[sKey.str()].as<sst::dict>()["amt"].as<sst::mint>();
      funds = funds.get() - 1;
      if(funds.get() == 0)
      {
         delete inven.asMap()[sKey.str()];
         inven.asMap().erase(sKey.str());
      }
   }

   void changeEquip(sst::dict& Char, std::string&, sst::dict&, size_t itemId, size_t itemType)
   {
      auto& slot = Char["equip"].as<sst::array>()[itemType].as<sst::mint>();
      slot = itemId;
   }
};

namespace { msgRegistrar<boostCharHandler> registrar("boostChar"); }

} // namespace server
