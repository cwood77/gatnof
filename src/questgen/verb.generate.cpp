#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../tcatlib/api.hpp"
#include "generator.hpp"
#include <memory>

namespace {

class genStagesCommand : public console::iCommand {
public:
   std::string oInPath;

   virtual void run(console::iLog& l);

private:
   void fillInProgression(sst::dict& d, const std::string& key, size_t max,
      std::vector<size_t>& v);
   void fillInOdds(sst::dict& d, const std::string& key, questgen::rarityOdds& o);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<genStagesCommand>("--genStages"));

      v->addParameter(
         console::stringParameter::required(offsetof(genStagesCommand,oInPath)));

      return v.release();
   }
} gVerb;

void genStagesCommand::run(console::iLog& l)
{
   tcat::typePtr<file::iFileManager> fMan;
   l.writeLnDebug("loading config settings (optional)");
   cmn::autoReleasePtr<file::iSstFile> pFile(&fMan->bindFile<file::iSstFile>(
      file::iFileManager::kExeAdjacent,
      "config.sst",
      file::iFileManager::kReadOnly
   ));
   pFile->tie(l);

   cmn::autoReleasePtr<file::iSstFile> pTmptFile(&fMan->bindFile<file::iSstFile>(
      file::iFileManager::kCurrentFolder,
      oInPath.c_str(),
      file::iFileManager::kReadOnly
   ));
   pTmptFile->tie(l);
   if(!pTmptFile->existed())
      throw std::runtime_error("template input file not found");

   // load the template
   using namespace questgen;
   stageData data(pTmptFile->dict()["num-stages"].as<sst::mint>().get());

   // fill in the progressions
   fillInProgression(pTmptFile->dict(),"enemy-party-size",5,data.partySize);
   fillInOdds(pTmptFile->dict(),"enemy-party-rarity",data.partyRarity);
   fillInProgression(pTmptFile->dict(),"enemy-party-level",60,data.partyLevel);
   fillInProgression(pTmptFile->dict(),"award-amts-1",1000,data.awardAmts1);

   // write
   stageGenerator gen;
   gen.tie(pFile->dict(),l);
   gen.writeToDisk(pTmptFile->dict()["num"].as<sst::mint>().get(),data);
}

void genStagesCommand::fillInProgression(sst::dict& d, const std::string& key, size_t max, std::vector<size_t>& v)
{
   auto& params = d[key].as<sst::dict>();
   questgen::intProgression().configure(
      params["start"].as<sst::mint>().get(),
      params["every-N-inc"].as<sst::mint>().get(),
      max,
      v
   );
}

void genStagesCommand::fillInOdds(sst::dict& d, const std::string& key, questgen::rarityOdds& o)
{
   auto& params = d[key].as<sst::array>();
   o.r = params[0].as<sst::mint>().get();
   o.sr = params[1].as<sst::mint>().get();
   o.ssr = params[2].as<sst::mint>().get();
   o.ur = params[3].as<sst::mint>().get();
}

} // anonymous namespace
