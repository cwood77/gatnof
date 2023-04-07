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
      //  - [DONE] new user login
      //  - [DONE] events (e.g. Easter)
      //  - [DONE] periodic while playing
      //
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

      // new user prize
      if(lastChk.get() == 0)
         bestowGems(inbox,"New user account",20000);

      // playtime awards
      auto playtime = now - login.get();
      playtimeAward(inbox,tstash,playtime,3,"3-sec playtime streak!",10);
      playtimeAward(inbox,tstash,playtime,4,"4-sec playtime streak!",10);
      playtimeAward(inbox,tstash,playtime,5,"5-sec playtime streak!",10);

      // scheduled game event awards
      auto& gameEvents = (*gServerData)["award-schedule"].as<sst::array>();
      for(size_t i=0;i<gameEvents.size();i++)
      {
         auto& evt = gameEvents[i].as<sst::dict>();
         auto& name = evt["name"].as<sst::str>().get();

         if(inRangeForEvent(evt,now) && !inRangeForEvent(evt,lastChk.get()))
            bestow(
               inbox,
               name,
               evt["amt"].as<sst::mint>().get(),
               evt["unit"].as<sst::str>().get()
            );
      }

      lastChk = now;
      ctxt.pAcct->flush();

      ch.sendSst(ctxt.pAcct->dict());
   }

private:
   void playtimeAward(sst::array& inbox, sst::dict& tstash, size_t playtime, size_t durationInSecs, const std::string& reason, size_t amt)
   {
      std::stringstream symbol;
      symbol << "update:playtime:" << durationInSecs;

      if(playtime > durationInSecs && !tstash.has(symbol.str()))
      {
         bestowGems(inbox,reason,amt);
         tstash.add<sst::tf>(symbol.str()) = true;
      }
   }

   bool inRangeForEvent(sst::dict& evt, time_t t)
   {
      return (evt["after"].as<sst::mint>().get() <= (size_t)t && (size_t)t < evt["before"].as<sst::mint>().get());
   }

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
