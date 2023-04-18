#include "../console/log.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <list>
#include <memory>

namespace server {

class summonHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      std::unique_ptr<sst::dict> pReq(ch.recvSst());
      std::unique_ptr<sst::dict> pSummonInfo(ch.recvSst());

      tcat::typePtr<db::iDict> dbDict;
      sst::dict result;
      std::string error;

      auto leftRight = (*pReq)["leftRight"].as<sst::mint>().get();
      auto upDown = (*pReq)["upDown"].as<sst::mint>().get();
      auto maxBuy = (*pSummonInfo)["max-buy"].as<sst::array>()[upDown].as<sst::mint>().get();
      result.add<sst::array>("pulls");
      auto gemsLeft = ctxt.pAcct->dict()["gems"].as<sst::mint>().get();

      size_t N = 0;
      if(leftRight == 0)
      {
         if(maxBuy < 1)
            error = "Insufficient funds";
         else
            N = 1;
      }
      else if(leftRight == 1)
      {
         if(maxBuy < 10)
            error = "Insufficient funds";
         else
            N = 10;
      }
      else if(leftRight == 2)
      {
         if(maxBuy < 1)
            error = "Insufficient funds";
         else
            N = maxBuy;
      }

      for(size_t i=0;i<N;i++)
      {
         if(upDown == 0)
         {
            awardPull(ctxt.pAcct->dict(),result,drawChar(*dbDict,drawRarityNormal()));
            gemsLeft -= 10;
         }
         else if(upDown == 1)
            ;
      }

      ch.sendString(error);
      ch.sendSst(result);
      ctxt.pAcct->dict()["gems"].as<sst::mint>() = gemsLeft; // write new gems to player!
      ch.sendSst(ctxt.pAcct->dict());
   }

private:
   db::rarities drawRarityNormal()
   {
      auto d = (::rand() % 100) + 1;
      if(d <=70)
         return db::kR;
      else if(d <=90)
         return db::kSr;
      else
         return db::kSsr;
   }

   size_t drawChar(db::iDict& d, db::rarities r)
   {
      const size_t n = d.numChars();
      while(true)
      {
         auto die = ::rand() % n;
         auto& c = d.findChar(die);
         if(c.rarity == r)
         {
            //log().writeLnDebug("summon drew char <%s> (%d)",c.name, (int)r);
            return die;
         }
      }
   }

   void awardPull(sst::dict& acct, sst::dict& results, size_t c)
   {
      auto& items = acct["items"].as<sst::dict>();
      auto& chars = acct["chars"].as<sst::dict>();

      auto& pulls = results["pulls"].as<sst::array>();
      std::stringstream sKey;
      sKey << c;

      auto& newPull = pulls.append<sst::dict>();
      newPull.add<sst::mint>("type") = c;

      if(chars.has(sKey.str()))
      {
         // add as a shard
         if(items.has(sKey.str()))
         {
            auto& cnt = items[sKey.str()].as<sst::dict>()["amt"].as<sst::mint>();
            cnt = cnt.get() + 1;
         }
         else
         {
            auto& noob = items.add<sst::dict>(sKey.str());
            noob.add<sst::mint>("type") = c;
            noob.add<sst::mint>("amt") = 1;
         }
         newPull.add<sst::tf>("shard") = true;
      }
      else
      {
         // add as a new char
         auto& noob = chars.add<sst::dict>(sKey.str());
         noob.add<sst::mint>("type") = c;

         // initial char stats
         noob.add<sst::mint>("level") = 1;
         noob.add<sst::mint>("stars") = 1;
         auto& equip = noob.add<sst::array>("equip");
         equip.append<sst::mint>() = 0;
         equip.append<sst::mint>() = 0;
         equip.append<sst::mint>() = 0;
         equip.append<sst::mint>() = 0;

         newPull.add<sst::tf>("shard") = false;
      }
   }
};

namespace { msgRegistrar<summonHandler> registrar("summon"); }

} // namespace server
