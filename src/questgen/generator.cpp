#include "../cmn/autoPtr.hpp"
#include "../cmn/misc.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../tcatlib/api.hpp"
#include "generator.hpp"

namespace questgen {

void intProgression::configure(size_t start, size_t everyNInc, size_t max, std::vector<size_t>& array)
{
   size_t v = start;
   size_t n = 0;
   for(size_t i=0;i<array.size();i++,n++)
   {
      if(everyNInc && everyNInc == n)
      {
         v++;
         if(v > max)
            v = max;
         n = 0;
      }
      array[i] = v;
   }
}

db::rarities rarityOdds::draw()
{
   size_t x = (::rand() % 100) + 1;
   if(x <= r)
      return db::kR;
   if(x <= (sr+r))
      return db::kSr;
   if(x <= (ssr+sr+r))
      return db::kSsr;
   else
      return db::kUr;
}

stageData::stageData(size_t numStages)
{
   partySize.resize(numStages);
   partyLevel.resize(numStages);
   awardAmts1.resize(numStages);
}

void stageGenerator::writeToDisk(size_t questNum, stageData& data)
{
   std::string outputPath = "data\\server\\quest\\00001.sst";

   tcat::typePtr<file::iFileManager> fMan;
   cmn::autoReleasePtr<file::iSstFile> pOutFile(&fMan->bindFile<file::iSstFile>(
      outputPath.c_str()
   ));
   pOutFile->tie(log());
   if(pOutFile->existed())
      log().writeLnInfo("WARNING: overwriting quest file at %s",outputPath.c_str());
   m_pDict = &(pOutFile->dict());

   writeBasics(questNum,data);
   writeStages(data);

   pOutFile->scheduleFor(file::iFileManager::kSaveOnClose);
}

void stageGenerator::writeBasics(size_t questNum, stageData& data)
{
   std::stringstream qNumStr;
   qNumStr << questNum;
   m_pDict->add<sst::str>("num") = qNumStr.str();
}

void stageGenerator::writeStages(stageData& data)
{
   m_pDict->add<sst::array>("stages");
   for(size_t i=0;i<data.partySize.size();i++)
      writeStage(i,data);
}

void stageGenerator::writeStage(size_t i, stageData& data)
{
   auto& stage = (*m_pDict)["stages"].as<sst::array>().append<sst::dict>();
   stage.add<sst::dict>("environs"); // empty for now

   writeAwards(stage,i,data);
   writeParty(stage,i,data);
}

void stageGenerator::writeAwards(sst::dict& stage, size_t i, stageData& data)
{
   auto& awards = stage.add<sst::array>("awards");

   auto& first = awards.append<sst::dict>();
   first.add<sst::str>("condition") = "win";
   first.add<sst::str>("unit") = "gold";
   first.add<sst::mint>("amt") = data.awardAmts1[i];
   first.add<sst::tf>("repeatable") = true;

   auto& second = awards.append<sst::dict>();
   second.add<sst::str>("condition") = "all-alive";
   second.add<sst::str>("unit") = "gold";
   second.add<sst::mint>("amt") = 5 * data.awardAmts1[i];
   second.add<sst::tf>("repeatable") = true;

   auto& third = awards.append<sst::dict>();
   third.add<sst::str>("condition") = "all-above-half";
   third.add<sst::str>("unit") = "gold";
   third.add<sst::mint>("amt") = 1;
   third.add<sst::tf>("repeatable") = false;
}

void stageGenerator::writeParty(sst::dict& stage, size_t i, stageData& data)
{
   auto& chars = stage.add<sst::dict>("chars");
   auto& lineUp = stage.add<sst::array>("line-up");
   stage.add<sst::mint>("line-up-bonus") = 0;
   std::set<size_t> used;

   for(size_t j=0;j<data.partySize[i];j++)
   {
      auto iChar = pickChar(used,data);

      std::stringstream sKey;
      sKey << iChar;
      auto& chEntry = chars.add<sst::dict>(sKey.str());
      chEntry.add<sst::mint>("type") = iChar;
      chEntry.add<sst::mint>("level") = 1;
      chEntry.add<sst::mint>("stars") = 1;
      auto& inven = chEntry.add<sst::array>("equip");
      inven.append<sst::mint>() = 0;
      inven.append<sst::mint>() = 0;
      inven.append<sst::mint>() = 0;
      inven.append<sst::mint>() = 0;

      lineUp.append<sst::mint>() = iChar;
   }
}

size_t stageGenerator::pickChar(std::set<size_t>& usedChars, stageData& data)
{
   tcat::typePtr<db::iDict> dbDict;
   auto r = data.partyRarity.draw();
   cmn::runawayLoopCheck lChk;
   while(true)
   {
      lChk.sanityCheck();
      auto iChar = (::rand() % dbDict->numChars());
      if(usedChars.find(iChar) != usedChars.end())
         continue;

      auto& ch = dbDict->findChar(iChar);
      if(ch.rarity != r)
         continue;

      usedChars.insert(iChar);
      return iChar;
   }
}

} // namespace questgen
