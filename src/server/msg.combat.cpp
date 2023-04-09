#include "../console/log.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include "message.hpp"
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
         c.insert(&ch);
   }

   db::Char& randomLiving()
   {
      std::vector<db::Char*> candidates = living();
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

   void setOutcome(bool victory)
   {
      m_d.add<sst::tf>("victory") = victory;
   }

private:
   sst::dict& m_d;
   sst::array *m_pArr;
};

} // namespace combat

class combatHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      tcat::typePtr<db::iDict> dbDict;
      sst::dict result;
      combat::recorder recorder(result);
      std::unique_ptr<sst::dict> pReq(ch.recvSst());

      // create db::Chars for each side
      combat::side pSide,oSide;
      buildSide(*dbDict,*pReq,pSide); // lie!
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

      ch.sendSst(result);
      ch.sendSst(ctxt.pAcct->dict());
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
      bool isPlayerAttacker = (&enemies == &player);

      // randomly select target from opposing side
      auto& target = targets.findOpposingSide(c).randomLiving();
      log().writeLnDebug(
         "[%s] (%s) attacks [%s]",
         c.name().c_str(),
         (isPlayerAttacker ? "(player)" : "(cpu)"),
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
      recorder.recordAttack(
         isPlayerAttacker,
         c.userData, target.userData,
         dmg, "" // TODO assumes no special attack
      );

      target.hp -= dmg;
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
};

namespace { msgRegistrar<combatHandler> registrar("combat"); }

} // namespace server
