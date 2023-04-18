#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>
#include <set>

namespace server {

class changeLineUpHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      std::unique_ptr<sst::dict> pReq(ch.recvSst());
      auto& reqLineUp = (*pReq)["line-up"].as<sst::array>();

      // make a copy of the current line-up
      // clear the line-up in the account SST
      std::vector<size_t> charVec;
      auto& lineUp = ctxt.pAcct->dict()["line-up"].as<sst::array>();
      while(lineUp.size())
      {
         charVec.push_back(lineUp[0].as<sst::mint>().get());
         log().writeLnTemp("curr lineup = %lld",lineUp[0].as<sst::mint>().get());
         lineUp.erase(0);
      }

      // rebuild the line-up array from req array
      for(size_t i=0;i<reqLineUp.size();i++)
      {
         auto idx = reqLineUp[i].as<sst::mint>().get();
         lineUp.append<sst::mint>() = charVec[idx];
         log().writeLnTemp("adding char [%lld] = %lld",idx,charVec[idx]);
      }

      // send back updated acct
      ch.sendSst(ctxt.pAcct->dict());
   }
};

namespace { msgRegistrar<changeLineUpHandler> registrar("changeLineUp"); }

} // namespace server
