#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
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

   l.writeLnDebug("compiling services");
   tcat::typePtr<cmn::serviceManager> svcMan;
   pen::object _pen(std::cout);
   cmn::autoService<pen::object> _penSvc(*svcMan,_pen);

   l.writeLnDebug("loading db");

   l.writeLnDebug("initializing cui");
   tcat::typePtr<cui::iFactory> sFac;

   l.writeLnDebug("contacting server for account info");
   tcat::typePtr<net::iNetProto> nProto;
   nProto->tie(pFile->dict(),l);
   cmn::autoReleasePtr<net::iAllocChannel> pAChan(&nProto->createPeerChannelClient(oServerAddr));
   cmn::autoReleasePtr<net::iChannel> pChan(&nProto->wrap(*pAChan.abdicate()));
   pChan->sendString("login");
   {
      sst::dict info;
      info.add<sst::str>("accountName") = oAccount;
      info.add<sst::mint>("version") = 0;
      pChan->sendSst(info);
   }
   std::unique_ptr<sst::dict> pAccount(pChan->recvSst());
   if(pAccount->has("error"))
      throw std::runtime_error((*pAccount)["error"].as<sst::str>().get().c_str());
   (*pAccount).add<sst::str>("accountName") = oAccount;
   (*pAccount).add<sst::str>("server-ip") = oServerAddr;
   cmn::autoService<std::unique_ptr<sst::dict> > _acnt(*svcMan,pAccount);

   l.writeLnDebug("switching to cui");
   pen::object::setupStdOut();
   _pen.str() << pen::showCursor(false);
   cmn::autoReleasePtr<cui::iLogic> pScr(&sFac->create<cui::iLogic>("home"));
   pScr->run();

   // return to normalcy
   _pen.str() << pen::fgcol(pen::kDefault) << pen::bgcol(pen::kDefault) << pen::moveTo(cui::pnt(1,22)) << pen::showCursor();
   pChan->sendString("logout");
}

} // anonymous namespace
