#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../tcatlib/api.hpp"
#include <memory>

namespace {

class playCommand : public console::iCommand {
public:
   std::string oServerAddr;
   std::string oAccount;
   std::string oPassword;

   virtual void run(console::iLog& l);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<playCommand>("--play"));

      v->addParameter(
         console::stringParameter::required(offsetof(playCommand,oServerAddr)));
      v->addParameter(
         console::stringParameter::required(offsetof(playCommand,oAccount)));
      v->addParameter(
         console::stringParameter::required(offsetof(playCommand,oPassword)));

      return v.release();
   }
} gVerb;

void playCommand::run(console::iLog& l)
{
   tcat::typePtr<file::iFileManager> fMan;
   l.writeLnDebug("loading config settings (optional)");
   cmn::autoReleasePtr<file::iSstFile> pFile(&fMan->bindFile<file::iSstFile>(
      file::iFileManager::kExeAdjacent,
      "config.sst",
      file::iFileManager::kReadOnly
   ));
   pFile->tie(l);

   l.writeLnDebug("loading db");

   l.writeLnDebug("initializing cui");

   l.writeLnDebug("contacting server for account info");

   l.writeLnDebug("switching to cui");
}

} // anonymous namespace
