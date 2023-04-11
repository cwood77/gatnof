#include "../../gen/screen/screen.precombat.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../shell/gameState.hpp"
#include "../tcatlib/api.hpp"
#include <memory>

namespace {

class logic : private precombat_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
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
         req.add<sst::mint>("quest#") = 1;
         req.add<sst::mint>("stage#") = 1;
         ch.sendSst(req);
      }
      std::unique_ptr<sst::dict> pCombatInfo(ch.recvSst());

      // disable some buttons
      m_upBtn.dim("unimpled");
      m_downBtn.dim("unimpled");
      m_lineUpBtn.dim("unimpled");
      m_goBtn.dim("unimpled");

      // whole screen re-draw
      render();

      // header
      m_aName.redraw(svcMan->demand<shell::gameState>().accountName);
      m_qNum.redraw(qNum);
      m_sNum.redraw(sNum);

      // opponent info
      auto teamBonus = (*pCombatInfo)["line-up-bonus"].as<sst::mint>().get();
      auto& opponentTeam = (*pCombatInfo)["line-up"].as<sst::array>();
      for(size_t i=0;i<opponentTeam.size();i++)
      {
         std::stringstream sKey;
         sKey << opponentTeam[i].as<sst::mint>().get();
         auto& overlay = (*pCombatInfo)["chars"].as<sst::dict>()[sKey.str()].as<sst::dict>();

         db::Char c(*dbDict,overlay,teamBonus);
         m_table[i].rChar.redraw(c.name());
         m_table[i].rElement.redraw(db::fmtElementsFixedWidth(c.element()));
      }

      // player info
      teamBonus = (*acct)["line-up-bonus"].as<sst::mint>().get();
      auto& playerTeam = (*acct)["line-up"].as<sst::array>();
      for(size_t i=0;i<playerTeam.size();i++)
      {
         std::stringstream sKey;
         sKey << playerTeam[i].as<sst::mint>().get();
         auto& overlay = (*acct)["chars"].as<sst::dict>()[sKey.str()].as<sst::dict>();

         db::Char c(*dbDict,overlay,teamBonus);
         m_table[i].lChar.redraw(c.name());
         m_table[i].lElement.redraw(db::fmtElementsFixedWidth(c.element()));
      }

      // determine advantage
      size_t minCnt = opponentTeam.size() < playerTeam.size()
         ? opponentTeam.size() : playerTeam.size();
      for(size_t i=0;i<minCnt;i++)
      {
         if(m_table[i].lElement.get() == "earth")
         {
            if(m_table[i].rElement.get() == "earth")
            {
               m_table[i].comp.redraw("=");
               m_table[i].compWord.setFormatMode(1);
               m_table[i].compWord.redraw(" equal");
            }
            else if(m_table[i].rElement.get() == "fire")
            {
               m_table[i].comp.redraw("<");
               m_table[i].compWord.setFormatMode(2);
               m_table[i].compWord.redraw(" weak ");
            }
            else if(m_table[i].rElement.get() == "water")
            {
               m_table[i].comp.redraw(">");
               m_table[i].compWord.setFormatMode(3);
               m_table[i].compWord.redraw("strong");
            }
         }
         else if(m_table[i].lElement.get() == "fire")
         {
            if(m_table[i].rElement.get() == "earth")
            {
               m_table[i].comp.redraw(">");
               m_table[i].compWord.setFormatMode(3);
               m_table[i].compWord.redraw("strong ");
            }
            else if(m_table[i].rElement.get() == "fire")
            {
               m_table[i].comp.redraw("=");
               m_table[i].compWord.setFormatMode(1);
               m_table[i].compWord.redraw(" equal");
            }
            else if(m_table[i].rElement.get() == "water")
            {
               m_table[i].comp.redraw("<");
               m_table[i].compWord.setFormatMode(2);
               m_table[i].compWord.redraw(" weak ");
            }
         }
         else if(m_table[i].lElement.get() == "water")
         {
            if(m_table[i].rElement.get() == "earth")
            {
               m_table[i].comp.redraw("<");
               m_table[i].compWord.setFormatMode(1);
               m_table[i].compWord.redraw(" weak ");
            }
            else if(m_table[i].rElement.get() == "fire")
            {
               m_table[i].comp.redraw(">");
               m_table[i].compWord.setFormatMode(3);
               m_table[i].compWord.redraw("strong");
            }
            else if(m_table[i].rElement.get() == "water")
            {
               m_table[i].comp.redraw("=");
               m_table[i].compWord.setFormatMode(2);
               m_table[i].compWord.redraw(" equal");
            }
         }
      }

      // handle user input
      cui::buttonHandler handler(m_error);
      handler.add(m_backBtn,[&](bool& stop){ stop = true; });
      handler.add(m_upBtn,[](bool&){});
      auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
      if(ans == &m_backBtn)
         return;
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("precombat") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
