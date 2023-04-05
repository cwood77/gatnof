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
#include <conio.h>
#include <memory>

namespace {

class testThread : public cmn::iThread {
public:
   virtual void run()
   {
      cmn::autoReleasePtr<net::iAllocChannel> pAChan(&pProto->createPeerChannelClient("localhost"));
      cmn::autoReleasePtr<net::iChannel> pChan(&pProto->wrap(*pAChan.abdicate()));

      for(size_t j=0;;j++)
      {
         pChan->sendString("a");
         pChan->recvString();

         if(i == 6 && (10 <= j && j <= 20))
         {
            ::printf("still alive\r\n");
         }
      }
   }

   size_t i;
   net::iNetProto *pProto;
};

class testCommand : public console::iCommand {
public:
   virtual void run(console::iLog& l);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<testCommand>("--test"));

      return v.release();
   }
} gVerb;

void testCommand::run(console::iLog& l)
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
   tcat::typePtr<net::iNetProto> nProto;
   nProto->tie(pFile->dict(),l);

   cmn::threadGroup<testThread> testers;

   for(size_t i=0;;i++)
   {
      l.writeLnDebug("waiting");
      ::Sleep(5000);

      auto& th = testers.allocate();
      th.i = i;
      th.pProto = &*nProto;
      testers.run(th);
   }
}

} // anonymous namespace
