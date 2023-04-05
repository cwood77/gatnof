#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
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
   }
};

namespace { msgRegistrar<loginHandler> registrar("login"); }

} // namespace server
