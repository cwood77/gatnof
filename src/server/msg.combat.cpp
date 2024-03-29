#include "../console/log.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
#include <functional>
#include <list>
#include <memory>

namespace server {
namespace combat {

class side {
public:
   std::list<db::Char> chars;

   template<class T>
   void addLiving(T& c)
   {
      for(auto& ch : chars)
         if(ch.hp > 0)
            c.insert(&ch);
   }

   db::Char& randomLiving()
   {
      std::vector<db::Char*> candidates = living();
      if(candidates.size() == 0)
         throw std::runtime_error("ISE");
      return *candidates[::rand() % candidates.size()];
   }

   bool isDefeated()
   {
      for(auto& ch : chars)
         if(ch.hp > 0)
            return false;
      return true;
   }

   std::vector<db::Char*> living()
   {
      std::vector<db::Char*> candidates;
      for(auto& ch : chars)
         if(ch.hp > 0)
            candidates.push_back(&ch);
      return candidates;
   }

   bool all(std::function<bool(db::Char&)> f)
   {
      for(auto& ch : chars)
         if(!f(ch))
            return false;
      return true;
   }
};

class targetTable {
public:
   void build(side& one, side& two)
   {
      buildOne(one,two);
      buildOne(two,one);
   }

   side& findOpposingSide(db::Char& c) { return *m_opponents[&c]; }

private:
   void buildOne(side& toEnum, side& enemy)
   {
      for(auto& c : toEnum.chars)
         m_opponents[&c] = &enemy;
   }

   std::map<db::Char*,side*> m_opponents;
};

class turnOrder {
public:
   typedef db::Char *type_t;
   bool operator()(const type_t& pLhs, const type_t& pRhs) const
   {
      return pLhs < pRhs;
   }
};

class recorder {
public:
   explicit recorder(sst::dict& d) : m_d(d)
   {
      m_pArr = &m_d.add<sst::array>("events");
      m_pAwards = &m_d.add<sst::array>("awards");
   }

   void recordAttack(bool player, size_t pIdx, size_t oIdx, long dmg,
      const std::string& spec)
   {
      auto& entry = m_pArr->append<sst::dict>();
      entry.add<sst::tf>("isPlayer") = player;
      entry.add<sst::mint>("pIdx") = pIdx;
      entry.add<sst::mint>("oIdx") = oIdx;
      entry.add<sst::mint>("dmg") = dmg;
      entry.add<sst::str>("spec") = spec;
   }

   void scoreAward(const std::string& condition, const std::string& boon, bool earned, bool alreadyHad)
   {
      auto& award = m_pAwards->append<sst::dict>();
      award.add<sst::str>("condition") = condition;
      award.add<sst::str>("boon") = boon;
      award.add<sst::tf>("earned") = earned;
      award.add<sst::tf>("alreadyHad") = alreadyHad;
   }

   void setOutcome(bool victory)
   {
      m_d.add<sst::tf>("victory") = victory;
   }

   bool wasVictory()
   {
      return m_d["victory"].as<sst::tf>().get();
   }

private:
   sst::dict& m_d;
   sst::array *m_pArr;
   sst::array *m_pAwards;
};

} // namespace combat

class combatHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      tcat::typePtr<db::iDict> dbDict;
      sst::dict result;
      combat::recorder recorder(result);
      auto questMoniker = ch.recvString();
      std::unique_ptr<sst::dict> pReq(ch.recvSst());

      // create db::Chars for each side
      combat::side pSide,oSide;
      buildSide(*dbDict,ctxt.pAcct->dict(),pSide);
      buildSide(*dbDict,*pReq,oSide);
      calculateIndivBonuses(*pReq,pSide,oSide);
      combat::targetTable targets;
      targets.build(pSide,oSide);

      bool done = false;
      while(!done)
      {
         // sort living chars by agility
         std::set<db::Char*,combat::turnOrder> turnOrder;
         pSide.addLiving(turnOrder);
         oSide.addLiving(turnOrder);

         for(auto *pChar : turnOrder)
         {
            // have char attack, if still alive
            if(pChar->hp == 0) continue;
            bool killed = false;
            takeTurn(*pChar,targets,pSide,killed,recorder);

            // if one side dead, done
            if(killed)
            {
               done = pSide.isDefeated();
               if(done)
                  recorder.setOutcome(false);
               else
               {
                  done = oSide.isDefeated();
                  if(done)
                     recorder.setOutcome(true);
               }

               if(done)
                  break;
            }
         }
      }

      log().writeLnTemp("scoring");
      scoreVictory(
         *pReq,
         ctxt.pAcct->dict(),questMoniker,
         recorder.wasVictory(),pSide,recorder);

      log().writeLnTemp("returning result SST");
      ch.sendSst(result);
      log().writeLnTemp("returning player SST");
      ch.sendSst(ctxt.pAcct->dict());
      log().writeLnTemp("returning");
   }

private:
   void buildSide(db::iDict& db, sst::dict& d, combat::side& s)
   {
      auto& charDict = d["chars"].as<sst::dict>();
      auto& lineUp = d["line-up"].as<sst::array>();
      auto bonus = d["line-up-bonus"].as<sst::mint>().get();
      for(size_t i=0;i<lineUp.size();i++)
      {
         std::stringstream stream;
         stream << lineUp[i].as<sst::mint>().get();
         auto& cOverlay = charDict[stream.str()].as<sst::dict>();
         s.chars.push_back(db::Char(db,cOverlay,bonus));
         s.chars.back().userData = i;
      }
   }

   void calculateIndivBonuses(sst::dict& d, combat::side& pSide, combat::side& oSide)
   {
      db::indivBonusCalculator calc(d["environs"].as<sst::dict>());
      std::vector<db::Char*> p = pSide.living();
      std::vector<db::Char*> o = oSide.living();
      size_t n = (p.size() < o.size() ? p.size() : o.size());
      for(size_t i=0;i<n;i++)
         calc.calculate(*p[i],*o[i]);
   }

   void takeTurn(db::Char& c, combat::targetTable& targets, combat::side& player, bool& killed, combat::recorder& recorder)
   {
      auto& enemies = targets.findOpposingSide(c);
      bool isPlayerAttacker = (&enemies != &player);

      // randomly select target from opposing side
      auto& target = enemies.randomLiving();
      log().writeLnDebug(
         "[%s] (%s) attacks [%s]",
         c.name().c_str(),
         (isPlayerAttacker ? "(cpu)" : "(player)"),
         target.name().c_str()
      );

      // -- randomly calculate dmg and adjust hp

      // base char stat is 1-200
      //    weapon stat is    20
      // special attack is    20
      //
      //                     240 max to start

      //               -1    0    1    2
      // bonuses are [x0.5  x1  x1.5  x2], cumulative but bounded
      //  - paired elemental
      //    +1
      //  - environmental (caste, subcaste)
      //    -1 or +1
      //  - team
      //    5/5 same subcaste: +2
      //
      //                     480 max after multiplier
      auto atk = c.atk(false); // TODO decide special
      auto def = target.def();

      // HP is always fixed at 100 max
      //
      // Subtract and lookup in a table for dmg
      //
      //        atk-def            dmg
      //   300 - 478  (op atk)   100 (all)
      //   175 - 299              90
      //   150 - 174              80
      //   125 - 149              70
      //   115 - 124              60
      //   100 - 114              50
      //    75 - 99               40
      //    50 - 74               30
      //     1 -  49              20
      //   -40 -   0  (even)      10
      //  -478 - -41  (op def)     0 (none)
      auto dmg = calculateDamage(atk,def);

      // add 0-9 randomly
      dmg += (::rand() % 10);

      log().writeLnDebug("DAMAGE is <%lld>",dmg);
      {
         std::stringstream narration;
         narration
            << c.name() << " attacks " << target.name() << " for " << dmg << " point(s)";
         recorder.recordAttack(
            isPlayerAttacker,
            c.userData, target.userData,
            dmg, narration.str() // TODO assumes no special attack
         );
      }

      if(dmg <= target.hp)
         target.hp -= dmg;
      else
         target.hp = 0;
      killed = (target.hp == 0);
   }

   size_t calculateDamage(size_t atk, size_t def)
   {
      long diff = atk-def;

           if(diff >= 300) return 100;
      else if(diff >= 175) return 90;
      else if(diff >= 150) return 80;
      else if(diff >= 125) return 70;
      else if(diff >= 115) return 60;
      else if(diff >= 100) return 50;
      else if(diff >=  75) return 40;
      else if(diff >=  50) return 30;
      else if(diff >=   1) return 20;
      else if(diff >= -40) return 10;
      else return 0;
   }

   bool alreadyGot(sst::dict& qHistory, const std::string& questMoniker, size_t iAward)
   {
      if(!qHistory.has(questMoniker))
         return false;

      auto& score = qHistory[questMoniker].as<sst::dict>()["score"].as<sst::array>();
      return score[iAward].as<sst::tf>().get();
   }

   void scoreVictory(sst::dict& questInfo, sst::dict& acct, const std::string& questMoniker, bool isVictory, combat::side& pSide, combat::recorder& r)
   {
      auto& qHistory = acct["quest-history"].as<sst::dict>();
      auto& awards = questInfo["awards"].as<sst::array>();
      std::vector<bool> earnings;
      for(size_t i=0;i<awards.size();i++)
      {
         auto& award = awards[i].as<sst::dict>();
         auto& condition = award["condition"].as<sst::str>().get();

         auto boon = buildBoonNarration(award);
         bool alreadyHad = alreadyGot(qHistory,questMoniker,i);
         bool earned = false;

         if(condition == "win")
         {
            earned = isVictory;
            r.scoreAward("defeat all enemies",boon,earned,alreadyHad);
         }
         else if(condition == "all-alive")
         {
            earned = pSide.all([](auto& c){ return c.hp > 0; });
            r.scoreAward("all members survive",boon,earned,alreadyHad);
         }
         else if(condition == "all-above-half")
         {
            earned = pSide.all([](auto& c){ return c.hp >= 50; });
            r.scoreAward("all members at least 50% health",boon,earned,alreadyHad);
         }
         else
            throw std::runtime_error("unsupported award condition");
         earnings.push_back(earned);

         if(earned && !alreadyHad)
            grantBoon(award,acct);
      }

      if(isVictory)
         updateQuestHistory(qHistory,questMoniker,earnings);
   }

   std::string buildBoonNarration(sst::dict& award)
   {
      auto& unit = award["unit"].as<sst::str>().get();
      std::stringstream boon;

      if(unit == "equip")
      {
         auto id = award["id"].as<sst::mint>().get();

         tcat::typePtr<db::iDict> dbDict;
         auto& e = dbDict->findItem(id);

         boon << e.name << " (" << db::fmtEquipTypes(e.type) << ")";
      }
      else
      {
         auto& amt = award["amt"].as<sst::mint>().get();
         boon << amt << " " << unit;
      }

      return boon.str();
   }

   void grantBoon(sst::dict& award, sst::dict& acct)
   {
      auto& unit = award["unit"].as<sst::str>().get();

      if(unit == "equip")
      {
         auto id = award["id"].as<sst::mint>().get();
         std::stringstream sKey;
         sKey << id;
         auto& inven = acct["items"].as<sst::dict>();
         if(inven.has(sKey.str()))
         {
            auto& cnt = inven[sKey.str()].as<sst::dict>()["amt"].as<sst::mint>();
            cnt = cnt.get() + 1;
         }
         else
         {
            auto& item = inven.add<sst::dict>(sKey.str());
            item.add<sst::mint>("amt") = 1;
            item.add<sst::mint>("type") = id;
         }
      }
      else if(unit == "gold")
      {
         auto& amt = award["amt"].as<sst::mint>().get();
         auto& field = acct["gold"].as<sst::mint>();
         field = field.get() + amt;
      }
      else
         throw std::runtime_error("unsupported award unit");
   }

   void updateQuestHistory(sst::dict& qHistory, const std::string& questMoniker, const std::vector<bool>& earnings)
   {
      if(!qHistory.has(questMoniker))
      {
         // new quest
         auto& thisQuest = qHistory.add<sst::dict>(questMoniker);
         auto& score = thisQuest.add<sst::array>("score");
         for(auto earned : earnings)
            score.append<sst::tf>() = earned;
      }
      else
      {
         // old quest
         auto& thisQuest = qHistory[questMoniker].as<sst::dict>();
         auto& score = thisQuest["score"].as<sst::array>();
         for(size_t i=0;i<earnings.size();i++)
            if(!score[i].as<sst::tf>().get())
               score[i].as<sst::tf>() = earnings[i];
      }
   }
};

namespace { msgRegistrar<combatHandler> registrar("combat"); }

} // namespace server
