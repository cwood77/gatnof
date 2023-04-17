#include "../../gen/screen/screen.battle.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/ani.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../shell/gameState.hpp"
#include "../tcatlib/api.hpp"

namespace {

class logic : private battle_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool)
   {
      tcat::typePtr<db::iDict> dbDict;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      auto& ch = svcMan->demand<net::iChannel>();
      auto& qNum = svcMan->demand<size_t>("selectedQuest");
      auto& sNum = svcMan->demand<size_t>("selectedStage");

      // query stage info
      ch.sendString("queryCombat");
      {
         sst::dict req;
         req.add<sst::str>("type") = "quest";
         req.add<sst::mint>("quest#") = qNum;
         req.add<sst::mint>("stage#") = sNum;
         req.add<sst::str>("mode") = "=";
         ch.sendSst(req);
      }
      ch.recvString();
      std::unique_ptr<sst::dict> pCombatInfo(ch.recvSst());
      delete ch.recvSst();

      // whole screen re-draw
      render();

      // header
      m_pAcct.redraw(svcMan->demand<shell::gameState>().accountName);

      // table - player chars
      {
         auto& lineUp = (*acct)["line-up"].as<sst::array>();
         for(size_t i=0;i<lineUp.size();i++)
         {
            auto& ch = dbDict->findChar(lineUp[i].as<sst::mint>().get());
            m_table[i].pName.redraw(ch.name);
            m_table[i].pGuage.redraw(100);
         }
      }
      // table - enemy chars
      {
         auto& lineUp = (*pCombatInfo)["line-up"].as<sst::array>();
         for(size_t i=0;i<lineUp.size();i++)
         {
            auto& ch = dbDict->findChar(lineUp[i].as<sst::mint>().get());
            m_table[i].oName.redraw(ch.name);
            m_table[i].oGuage.redraw(100);
         }
      }

      // run the combat on the server
      ch.sendString("combat");
      ch.sendSst(*pCombatInfo);
      std::unique_ptr<sst::dict> pBattleDetails(ch.recvSst());
      acct.reset(ch.recvSst());

      // play back the battle details
      ani::delay d;
      d.nMSec = 1;
      auto& evts = (*pBattleDetails)["events"].as<sst::array>();
      for(size_t i=0;i<evts.size();i++)
      {
         auto& evt = evts[i].as<sst::dict>();
         m_error.update(evt["spec"].as<sst::str>().get());
         auto isPlayer = evt["isPlayer"].as<sst::tf>().get();

         ani::flipbook fb(d);
         ani::sequencer seq(fb);
         seq.simultaneous(
         {
            // lasso outline
            [&](auto& c)
            {
               auto& row = m_table[evt["pIdx"].as<sst::mint>().get()];
               cui::pnt pnt = isPlayer ? row.pName.getLoc() : row.oName.getLoc();
               pnt.x-=2;
               pnt.y-=1;
               ani::outliner().outline(pnt,46,3,c);
            },
            // blink (manually)
            [&](auto& c)
            {
               auto& row = m_table[evt["oIdx"].as<sst::mint>().get()];
               cui::pnt pnt = isPlayer ? row.oName.getLoc() : row.pName.getLoc();
               pnt.x-=2;
               pnt.y-=1;
               ani::prim::box(c.getFrame(0),pnt,46,3,pen::kMagenta);
               ani::prim::box(c.getFrame(30),pnt,46,3,pen::kBlue);
               ani::prim::box(c.getFrame(60),pnt,46,3,pen::kMagenta);
               ani::prim::box(c.getFrame(120),pnt,46,3,pen::kBlue);
            }
         });
         seq.simultaneous(
         {
            // clear
            [&](auto& c)
            {
               auto& row = m_table[evt["pIdx"].as<sst::mint>().get()];
               cui::pnt pnt = isPlayer ? row.pName.getLoc() : row.oName.getLoc();
               pnt.x-=2;
               pnt.y-=1;
               ani::prim::box(c.getFrame(0),pnt,46,3,pen::kBlue);
            },
            [&](auto& c)
            {
               auto& row = m_table[evt["oIdx"].as<sst::mint>().get()];
               cui::pnt pnt = isPlayer ? row.oName.getLoc() : row.pName.getLoc();
               pnt.x-=2;
               pnt.y-=1;
               ani::prim::box(c.getFrame(0),pnt,46,3,pen::kBlue);
            }
         });

         auto& pn = svcMan->demand<pen::object>();
         ani::delayTweakKeystrokeMonitor(d).run([&](){ fb.run(pn); });

         // update guage
         auto& row = m_table[evt["oIdx"].as<sst::mint>().get()];
         auto& g = isPlayer ?
            (cui::guageControl&)row.oGuage : (cui::guageControl&)row.pGuage;
         int dmg = evt["dmg"].as<sst::mint>().get();
         int noob = g.get() - dmg;
         if(g.get() < dmg)
            noob = 0;
         g.update(noob);
         if(noob == 0)
         {
            auto& n = isPlayer ?
               (cui::stringControl&)row.oName : (cui::stringControl&)row.pName;
            n.setFormatMode(2);
            n.redraw(n.get());
         }
      }
      auto& advQuest = svcMan->demand<bool>("advQuest");
      if((*pBattleDetails)["victory"].as<sst::tf>().get())
      {
         m_error.update("Victory");

         tcat::typePtr<cui::iFactory> sFac;
         cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("winBattle"));

         cmn::autoService<sst::dict> _battleDetailsSvc(*svcMan,*pBattleDetails,"battleDetails");
         pL->run();
         advQuest = true;
      }
      else
      {
         m_error.update("Failure");

         tcat::typePtr<cui::iFactory> sFac;
         cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("loseBattle"));
         pL->run();
         advQuest = false;
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("battle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
