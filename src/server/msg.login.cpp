#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>

namespace server {

class loginHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      log().writeLnTemp("here");

      std::unique_ptr<sst::dict> pInfo(ch.recvSst());

      log().writeLnVerbose("looking up account '%s'",(*pInfo)["accountName"].as<sst::str>().get().c_str());

      // locate file on disk and bind it; stash in context
      std::string path = "C:\\cygwin64\\home\\chris\\dev\\gatnof\\data\\server\\acct\\"; // TODO
      path += (*pInfo)["accountName"].as<sst::str>().get();
      path += ".sst";

      tcat::typePtr<file::iFileManager> fMan;
      if(!fMan->doesFileExist(path))
      {
         sst::dict error;
         error.add<sst::str>("error") = "account does not exist";
         ch.sendSst(error);
      }
      else
      {
         if(ctxt.pAcct)
         {
            ctxt.pAcct->release();
            ctxt.pAcct = NULL;
         }

         ctxt.pAcct = &fMan->bindFile<file::iSstFile>(path.c_str());
         ctxt.pAcct->scheduleFor(file::iFileManager::kSaveOnClose);

         // rotate timestamps
         {
            auto& curr = ctxt.pAcct->dict()["ts-curr-login"].as<sst::mint>();
            ctxt.pAcct->dict().add<sst::mint>("ts-prev-login") = curr.get();
            curr = ::time(NULL);
         }

         // wipe stash
         ctxt.pAcct->dict().add<sst::dict>("svr-tmp-stash");

         ch.sendSst(ctxt.pAcct->dict());
      }
   }
};

namespace { msgRegistrar<loginHandler> registrar("login"); }

} // namespace server
