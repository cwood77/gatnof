#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../cmn/win32.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "connectionThread.hpp"
#include "listenThread.hpp"
#include <conio.h>
#include <memory>

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
   cmn::osEvent stopSignal;
   tcat::typePtr<net::iNetProto> nProto;
   nProto->tie(pFile->dict(),l);

   l.writeLnVerbose("create and start listener");
   cmn::threadGroup<server::connectionThread> workers;
   server::listenThread listener(*nProto,stopSignal,workers);
   listener.tie(pFile->dict(),l);
   cmn::threadController listenerTc(listener);
   listenerTc.start();

   // in a listener thread
   //   create channel
   //   recvString command
   //     hand off to threadpool

   ::getch();
   l.writeLnVerbose("stopping and joining threads");
   stopSignal.set();
   listenerTc.join();
}

} // anonymous namespace
