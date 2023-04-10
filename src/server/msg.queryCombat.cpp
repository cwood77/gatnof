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

      std::string path = "C:\\cygwin64\\home\\chris\\dev\\gatnof\\data\\server\\quest\\"; // TODO
      {
         auto& questNum = (*pReq)["quest#"].as<sst::mint>().get();
         char buffer[1024];
         ::sprintf(buffer,"%05lld",questNum);
         path += buffer;
         path += ".sst";
      }

      tcat::typePtr<file::iFileManager> fMan;
      if(!fMan->doesFileExist(path))
         throw std::runtime_error("quests not found");

      cmn::autoReleasePtr<file::iSstFile> pFile(
         &fMan->bindFile<file::iSstFile>(path.c_str(),
         file::iFileManager::kReadOnly));

      auto& stageNum = (*pReq)["stage#"].as<sst::mint>().get();
      auto& stage = pFile->dict()["stages"].as<sst::array>()[stageNum-1].as<sst::dict>();

      ch.sendSst(stage);
   }
};

namespace { msgRegistrar<queryCombatHandler> registrar("queryCombat"); }

} // namespace server
