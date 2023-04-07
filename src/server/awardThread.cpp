#include "../console/log.hpp"
#include "awardThread.hpp"
#include "data.hpp"

namespace server {

void awardThread::run()
{
   while(true)
   {
      // setup for today
      auto now = ::time(NULL);
      auto today = normalizeToDaily(now);
      auto tmw = normalizeToDaily(today + 25*60*60 + 60); // add 25 hours and 1 min
      onceDaily(today);

      while(true)
      {
         // figure when is tomorrow
         now = ::time(NULL);
         if(now >= tmw)
            break;
         DWORD mSecTillTomorrow = (tmw - now) * 1000;

         // sleep until then
         auto rval = ::WaitForSingleObject(m_stopSignal.getHandle(),mSecTillTomorrow);
         if(rval == WAIT_OBJECT_0)
            return;

         // recheck (allow for timing error)
      }
   }
}

time_t awardThread::normalizeToDaily(time_t t)
{
   struct tm *pTm = ::localtime(&t);
   pTm->tm_sec   = 0;
   pTm->tm_min   = 0;
   pTm->tm_hour  = 0;
   pTm->tm_wday  = 0;
   pTm->tm_yday  = 0;
   pTm->tm_isdst = -1;
   return ::mktime(pTm);
}

void awardThread::onceDaily(time_t normalizedT)
{
   // grab lock
   cmn::autoLock _lock(*gDataLock);

   // remove dailies from schedule (if any)
   auto& sched = (*gServerData)["award-schedule"].as<sst::array>();
   if(sched.size())
   {
      // the daily is always the last and has a unlisted field
      auto& entry = sched[sched.size()-1].as<sst::dict>();
      if(entry.has("byAwardThread"))
      {
         log().writeLnDebug("removing last daily schedule");
         sched.erase(sched.size()-1);
      }
   }

   // add today's daily to schedule
   log().writeLnDebug("adding today's daily schedule");
   auto& noob = sched.append<sst::dict>();
   noob.add<sst::tf>("byAwardThread") = true;

   noob.add<sst::mint>("after") = normalizedT;
   noob.add<sst::mint>("before") = normalizedT + 24*60*60;
   noob.add<sst::str>("name") = "Daily gems";
   noob.add<sst::mint>("amt") = 10;
   noob.add<sst::str>("unit") = "gems";
}

} // namespace server
