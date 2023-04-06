#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>

namespace server {

class getHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      auto& inbox = ctxt.pAcct->dict()["inbox"].as<sst::array>();
      auto& prize = inbox[0].as<sst::dict>();
      auto& unit = prize["unit"].as<sst::str>().get();
      if(unit == "gems")
      {
         ctxt.pAcct->dict()["gems"].as<sst::mint>() = (
            ctxt.pAcct->dict()["gems"].as<sst::mint>().get()
            + prize["amt"].as<sst::mint>().get()
         );
         inbox.erase(0);
      }
      else
      {
         log().writeLnInfo("unknown prize unit <%s>",unit.c_str());
         log().writeLnInfo("DROPPING CONNECTION");
         ctxt.quit = true;
         return;
      }

      ctxt.pAcct->flush();

      ch.sendSst(ctxt.pAcct->dict());
   }
};

namespace { msgRegistrar<getHandler> registrar("get"); }

} // namespace server
