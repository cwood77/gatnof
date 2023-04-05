#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>

namespace server {

class updateHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      time_t now = ::time(NULL);
      auto& lastChk = ctxt.pAcct->dict()["last-update-check"].as<sst::mint>();
      auto& inbox = ctxt.pAcct->dict()["inbox"].as<sst::array>();
      if(lastChk.get() == 0)
      {
         // new user!
         auto& present = inbox.append<sst::dict>();
         present.add<sst::str>("reason") = "New user account";
         present.add<sst::str>("unit") = "gems";
         present.add<sst::mint>("amt") = 20000;
      }
      else if((now - lastChk.get()) >= (20*60)) // 20min // needs be login time
      {
         // activity present
         auto& present = inbox.append<sst::dict>();
         present.add<sst::str>("reason") = "Gem accural";
         present.add<sst::str>("unit") = "gems";
         present.add<sst::mint>("amt") = 100;
      }

      lastChk = now;
      ctxt.pAcct->flush();

      ch.sendSst(ctxt.pAcct->dict());
   }
};

namespace { msgRegistrar<updateHandler> registrar("update"); }

} // namespace server
