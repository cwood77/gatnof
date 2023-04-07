#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "data.hpp"
#include "message.hpp"
#include <memory>

namespace server {

class updateHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      // final and master list of prizes I want to support
      //
      // -- calculated during update, without award thread
      // prev-login vs. 0
      //  - [DONE] new user login
      // curr-login vs. P
      //  - [in work] periodic while playing
      // last-update vs. date
      //  - [in work] events (e.g. Easter)
      // ts-update, prev-login, curr-login
      //  - 3 days login streak
      // last-update
      //  - downtime compensation
      //
      // -- calculated during update, using award thread
      // thread timer, prev-login, curr-login
      //    maybe do this on time of global data above?
      //  - daily at a certain time, if logged in w/i 5 days
      //
      // -- elsewhere
      // awarded when that happens, elsewhere
      //  - when you get X chars

      time_t now = ::time(NULL);
      auto& lastChk = ctxt.pAcct->dict()["ts-update"].as<sst::mint>();
      auto& login = ctxt.pAcct->dict()["ts-curr-login"].as<sst::mint>();
      auto& inbox = ctxt.pAcct->dict()["inbox"].as<sst::array>();
      auto& tstash = ctxt.pAcct->dict()["svr-tmp-stash"].as<sst::dict>();
      auto& pstash = ctxt.pAcct->dict()["svr-stash"].as<sst::dict>();

      // new user prize
      if(lastChk.get() == 0)
         bestowGems(inbox,"New user account",20000);

      // playtime awards
      auto playtime = now - login.get();
      if(playtime > 3 && !tstash.has("update-playtime"))
      {
         bestowGems(inbox,"Playtime streak!",20);
         tstash.add<sst::str>("update-playtime");
      }

      // scheduled game event awards
      auto& gameEvents = (*gServerData)["award-schedule"].as<sst::array>();
      for(size_t i=0;i<gameEvents.size();i++)
      {
         auto& evt = gameEvents[i].as<sst::dict>();
         auto& name = evt["name"].as<sst::str>().get();

         if((size_t)now >= evt["after"].as<sst::mint>().get())
         {
            if((size_t)now < evt["before"].as<sst::mint>().get())
            {
               if(!pstash.has(name))
               {
                  bestow(
                     inbox,
                     name,
                     evt["amt"].as<sst::mint>().get(),
                     evt["unit"].as<sst::str>().get()
                  );
                  pstash.add<sst::str>(name);
               }
            }
            else
            {
               // tidy the stash
               if(pstash.has(name))
               {
                  auto& m = pstash.asMap();
                  delete m[name];
                  m.erase(name);
               }
            }
         }
      }

      lastChk = now;
      ctxt.pAcct->flush();

      ch.sendSst(ctxt.pAcct->dict());
   }

private:
   void bestowGems(sst::array& inbox, const std::string& reason, size_t amt)
   {
      bestow(inbox,reason,amt,"gems");
   }

   void bestow(sst::array& inbox, const std::string& reason, size_t amt, const std::string& unit)
   {
      auto& present = inbox.append<sst::dict>();
      present.add<sst::str>("reason") = reason;
      present.add<sst::mint>("amt") = amt;
      present.add<sst::str>("unit") = unit;
   }
};

namespace { msgRegistrar<updateHandler> registrar("update"); }

} // namespace server
