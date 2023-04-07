#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../cmn/win32.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "awardThread.hpp"
#include "data.hpp"
#include "connectionThread.hpp"
#include "listenThread.hpp"
#include <conio.h>
#include <memory>

namespace server { sst::dict *gServerData = NULL; }

namespace {

class listenCommand : public console::iCommand {
public:
   virtual void run(console::iLog& l);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<listenCommand>("--listen"));

      return v.release();
   }
} gVerb;

void listenCommand::run(console::iLog& l)
{
   tcat::typePtr<file::iFileManager> fMan;
   l.writeLnDebug("loading config settings (optional)");
   cmn::autoReleasePtr<file::iSstFile> pFile(&fMan->bindFile<file::iSstFile>(
      file::iFileManager::kExeAdjacent,
      "config.sst",
      file::iFileManager::kReadOnly
   ));
   pFile->tie(l);

   l.writeLnVerbose("starting up");
   // needs to be a manual event to release multiple things
   cmn::osEvent stopSignal("",false);
   tcat::typePtr<net::iNetProto> nProto;
   nProto->tie(pFile->dict(),l);

   l.writeLnVerbose("load global game data");
   cmn::autoReleasePtr<file::iSstFile> pData(&fMan->bindFile<file::iSstFile>(
      "C:\\cygwin64\\home\\chris\\dev\\gatnof\\data\\server\\global.sst", // TODO
      file::iFileManager::kReadOnly
   ));
   pData->tie(l);
   server::gServerData = &pData->dict();

   l.writeLnVerbose("create and start listener threads");
   cmn::threadGroup<server::connectionThread> workers;
   server::listenThread listener(*nProto,stopSignal,workers);
   listener.tie(pFile->dict(),l);
   cmn::threadController listenerTc(listener);
   listenerTc.start();

   ::getch();
   l.writeLnVerbose("stopping and joining threads threads");
   stopSignal.set();
   listenerTc.join();
   workers.join();
   l.writeLnVerbose("bye");
}

} // anonymous namespace
