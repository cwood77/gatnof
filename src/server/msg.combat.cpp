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
      std::vector<db::Char*> candidates;
      for(auto& ch : chars)
         if(ch.hp > 0)
            candidates.push_back(&ch);
      return *candidates[::rand() % candidates.size()];
   }

   bool isDefeated()
   {
      for(auto& ch : chars)
         if(ch.hp > 0)
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

} // namespace combat

class combatHandler : public iMsgHandler {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt)
   {
      tcat::typePtr<db::iDict> dbDict;

      std::unique_ptr<sst::dict> pReq(ch.recvSst());

      // create db::Chars for each side
      combat::side pSide,oSide;
      buildSide(*dbDict,*pReq,pSide); // lie!
      buildSide(*dbDict,*pReq,oSide);
      combat::targetTable targets;
      targets.build(pSide,oSide);

      bool done = false;
      while(!done)
      {
         // do
         //   sort living chars by aglity
         std::set<db::Char*,combat::turnOrder> turnOrder;
         pSide.addLiving(turnOrder);
         oSide.addLiving(turnOrder);

         for(auto *pChar : turnOrder)
         {
            //   foreach sorted
            //     have char attack, if still alive
            if(pChar->hp == 0) continue;
            bool killed = false;
            takeTurn(*pChar,targets,pSide,killed);

            //     if one side dead, done
            if(killed)
            {
               done = (pSide.isDefeated() || oSide.isDefeated());
               if(done)
                  break;
            }
         }
      }

      ch.sendSst(ctxt.pAcct->dict());
   }

private:
   void buildSide(db::iDict& db, sst::dict& d, combat::side& s)
   {
      auto& charDict = d["chars"].as<sst::dict>();
      auto& lineUp = d["line-up"].as<sst::array>();
      for(size_t i=0;i<lineUp.size();i++)
      {
         std::stringstream stream;
         stream << lineUp[i].as<sst::mint>().get();
         auto& cOverlay = charDict[stream.str()].as<sst::dict>();
         s.chars.push_back(db::Char(db,cOverlay));
      }
   }

   void takeTurn(db::Char& c, combat::targetTable& targets, combat::side& player, bool& killed)
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

      // randomly calculate dmg and adjust hp
      target.hp--;

      killed = (target.hp == 0);
   }
};

namespace { msgRegistrar<combatHandler> registrar("combat"); }

} // namespace server
