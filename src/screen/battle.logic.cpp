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
         ch.sendSst(req);
      }
      std::unique_ptr<sst::dict> pCombatInfo(ch.recvSst());

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
            m_table[i].pGuage.redraw(std::string(10,'-'));
         }
      }
      // table - enemy chars
      {
         auto& lineUp = (*pCombatInfo)["line-up"].as<sst::array>();
         for(size_t i=0;i<lineUp.size();i++)
         {
            auto& ch = dbDict->findChar(lineUp[i].as<sst::mint>().get());
            m_table[i].oName.redraw(ch.name);
            m_table[i].oGuage.redraw(std::string(10,'-'));
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
         // clear
         seq.simultaneous(
         {
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
         fb.run(pn);
      }
      if((*pBattleDetails)["victory"].as<sst::tf>().get())
         m_error.update("Victory");
      else
         m_error.update("Failure");

      tcat::typePtr<cui::iUserInput> in;
      in->getKey();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("battle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
