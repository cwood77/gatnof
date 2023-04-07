#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../tcatlib/api.hpp"
#include "data.hpp"
#include <memory>

namespace {

class compileCommand : public console::iCommand {
public:
   virtual void run(console::iLog& l);

private:
   void calculateDateTime(sst::dict& d, const std::string& key);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<compileCommand>("--compile"));

      return v.release();
   }
} gVerb;

void compileCommand::run(console::iLog& l)
{
   tcat::typePtr<file::iFileManager> fMan;
   l.writeLnDebug("loading config settings (optional)");
   cmn::autoReleasePtr<file::iSstFile> pFile(&fMan->bindFile<file::iSstFile>(
      file::iFileManager::kExeAdjacent,
      "config.sst",
      file::iFileManager::kReadOnly
   ));
   pFile->tie(l);

   l.writeLnVerbose("load global game data");
   cmn::autoReleasePtr<file::iSstFile> pData(&fMan->bindFile<file::iSstFile>(
      "C:\\cygwin64\\home\\chris\\dev\\gatnof\\data\\server\\global.sst", // TODO
      file::iFileManager::kSaveOnClose
   ));
   pData->tie(l);
   server::gServerData = &pData->dict();

   // calcuate after/before for awards
   auto& gameEvents = (*server::gServerData)["award-schedule"].as<sst::array>();
   for(size_t i=0;i<gameEvents.size();i++)
   {
      auto& evt = gameEvents[i].as<sst::dict>();
      calculateDateTime(evt,"before");
      calculateDateTime(evt,"after");
   }
}

void compileCommand::calculateDateTime(sst::dict& d, const std::string& key)
{
   if(d.has(key)) return;

   auto& str = d[key + "-str"].as<sst::str>().get();

   struct tm t;
   ::memset(&t,0,sizeof(struct tm));
   ::sscanf(str.c_str(),"%d-%d-%d %d-%d-%d",
      &t.tm_year,
      &t.tm_mon,
      &t.tm_mday,
      &t.tm_hour,
      &t.tm_min,
      &t.tm_sec
   );

   // adjust b/c the C spec is dumb
   t.tm_year -= 1900;
   t.tm_mon -= 1;

   auto itime = ::mktime(&t);
   d.add<sst::mint>(key) = itime;

   {
      // diagnostics
      char block[1024];
      ::strftime(block,1023,"%b %d %Y - %H:%M:%S",&t);
      ::printf("rebuilt time <%s> as <%s>\r\n",str.c_str(),block);
   }
}

} // anonymous namespace
