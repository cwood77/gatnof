#include "../file/manager.hpp"
#include "message.hpp"

namespace server {

class logoutHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      if(ctxt.pAcct)
      {
         ctxt.pAcct->release();
         ctxt.pAcct = NULL;
      }
      ctxt.quit = true;
   }
};

namespace { msgRegistrar<logoutHandler> registrar("logout"); }

} // namespace server
