#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <memory>

namespace server {

class queryCombatHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      std::unique_ptr<sst::dict> pReq(ch.recvSst());
      if((*pReq)["type"].as<sst::str>().get() != "quest")
         throw std::runtime_error("only quests are supported");
      auto qNum = (*pReq)["quest#"].as<sst::mint>().get();
      auto sNum = (*pReq)["stage#"].as<sst::mint>().get();

      while(true)
      {
         std::string path = "C:\\cygwin64\\home\\chris\\dev\\gatnof\\data\\server\\quest\\"; // TODO
         {
            char buffer[1024];
            ::sprintf(buffer,"%05lld",qNum);
            path += buffer;
            path += ".sst";
         }

         tcat::typePtr<file::iFileManager> fMan;
         if(!fMan->doesFileExist(path))
         {
            ch.sendString("no more quests");
            return;
         }

         cmn::autoReleasePtr<file::iSstFile> pFile(
            &fMan->bindFile<file::iSstFile>(path.c_str(),
            file::iFileManager::kReadOnly));

         auto& stages = pFile->dict()["stages"].as<sst::array>();
         if(sNum == 0)
            sNum = stages.size() - 1;

         // handle wrapping
         auto& mode = (*pReq)["mode"].as<sst::str>();
         if(mode.get() == "++")
         {
            mode = "=";
            sNum++;
            if((sNum-1) >= stages.size())
            {
               qNum++;
               sNum = 0;
               continue;
            }
         }
         else if(mode.get() == "--")
         {
            mode = "=";
            sNum--;
            if(sNum == 0)
            {
               if(qNum == 1)
               {
                  ch.sendString("no more quests");
                  return;
               }

               qNum--;
               sNum = 0; // pick a stage later
               continue;
            }
         }

         auto& stage = stages[sNum-1].as<sst::dict>();
         ch.sendString("");
         ch.sendSst(stage);

         sst::dict newNums;
         newNums.add<sst::mint>("quest#") = qNum;
         newNums.add<sst::mint>("stage#") = sNum;
         ch.sendSst(newNums);
         break;
      }
   }
};

namespace { msgRegistrar<queryCombatHandler> registrar("queryCombat"); }

} // namespace server
