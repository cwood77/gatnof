#include "message.hpp"

namespace server {

class logoutHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      // release file ptr on context
      ctxt.quit = true;
   }
};

namespace { msgRegistrar<logoutHandler> registrar("logout"); }

} // namespace server
