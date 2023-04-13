#ifndef ___questgen_generator___
#define ___questgen_generator___

#include "../cmn/subobject.hpp"
#include "../db/api.hpp"
#include <set>
#include <vector>

namespace questgen {

class intProgression {
public:
   void configure(size_t start, size_t everyNInc, size_t max, std::vector<size_t>& array);
};

class rarityOdds {
public:
   rarityOdds() : r(0), sr(0), ssr(0), ur(0) {}

   size_t r;
   size_t sr;
   size_t ssr;
   size_t ur;

   db::rarities draw();
};

class stageData {
public:
   explicit stageData(size_t numStages);

   std::vector<size_t> partySize;
   rarityOdds          partyRarity;
   std::vector<size_t> partyLevel;
   std::vector<size_t> awardAmts1;
};

class stageGenerator : public cmn::subobject {
public:
   void writeToDisk(size_t questNum, stageData& data);

private:
   void writeBasics(size_t questNum, stageData& data);
   void writeStages(stageData& data);
   void writeStage(size_t i, stageData& data);
   void writeAwards(sst::dict& stage, size_t i, stageData& data);
   void writeParty(sst::dict& stage, size_t i, stageData& data);
   size_t pickChar(std::set<size_t>& usedChars, stageData& data);

   sst::dict *m_pDict;
};

} // namespace questgen

#endif // ___questgen_generator___
