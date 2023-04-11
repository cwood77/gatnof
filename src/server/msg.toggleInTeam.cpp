#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"

namespace server {

class toggleInTeamHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      std::string sCharId = ch.recvString();
      int charId = 0;
      ::sscanf(sCharId.c_str(),"%d",&charId);

      bool removed = false;
      auto& lineUp = ctxt.pAcct->dict()["line-up"].as<sst::array>();
      for(size_t i=0;i<lineUp.size();i++)
      {
         if(lineUp[i].as<sst::mint>().get() == (size_t)charId)
         {
            lineUp.erase(i);
            removed = true;
            break;
         }
      }

      std::string error;
      if(!removed)
      {
         if(lineUp.size() == 5)
            error = "Not enough room in team";
         else
            lineUp.append<sst::mint>() = charId;
      }

      if(error.empty())
      {
         tcat::typePtr<db::iDict> dbDict;
         db::teamBonusCalculator calc;
         for(size_t i=0;i<lineUp.size();i++)
            calc.addChar(dbDict->findChar(lineUp[i].as<sst::mint>().get()));
         ctxt.pAcct->dict()["line-up-bonus"].as<sst::mint>() = calc.calculate();
      }

      ch.sendString(error);
      if(error.empty())
         ch.sendSst(ctxt.pAcct->dict());
   }
};

namespace { msgRegistrar<toggleInTeamHandler> registrar("toggleInTeam"); }

} // namespace server

