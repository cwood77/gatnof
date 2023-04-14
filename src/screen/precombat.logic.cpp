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

   virtual void run(bool)
   {
      tcat::typePtr<db::iDict> dbDict;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      auto& ch = svcMan->demand<net::iChannel>();
      auto& qNum = svcMan->demand<size_t>("selectedQuest");
      auto& sNum = svcMan->demand<size_t>("selectedStage");

      // whole screen re-draw
      render();

      std::unique_ptr<sst::dict> pCombatInfo;
      while(true)
      {
         // query stage info
         ch.sendString("queryCombat");
         {
            sst::dict req;
            req.add<sst::str>("type") = "quest";
            req.add<sst::mint>("quest#") = qNum;
            req.add<sst::mint>("stage#") = sNum;
            ch.sendSst(req);
         }
         std::string rsp = ch.recvString();
         if(rsp == "no-quest")
         {
            qNum++;
            sNum=1;
            continue;
         }
         else if(!rsp.empty())
            m_error.redraw(rsp);
         else
            pCombatInfo.reset(ch.recvSst());

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
         for(size_t i=opponentTeam.size();i<5;i++)
         {
            m_table[i].rChar.redraw("");
            m_table[i].rElement.redraw("");
            m_table[i].comp.redraw("");
            m_table[i].compWord.setFormatMode(1);
            m_table[i].compWord.redraw("");
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
            m_table[i].lChar.rightJustify = true;
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
         handler.add(m_upBtn,[&](bool& stop)
         {
            if(qNum == 1)
            {
               if(sNum != 1)
               {
                  sNum--;
                  stop = true;
               }
            }
            else // qNum > 1
            {
               if(sNum != 1)
               {
                  sNum--;
                  stop = true;
               }
               else // sNum == 1
               {
                  qNum--;
                  sNum = 99;
                  stop = true;
               }
            }
         });
         handler.add(m_downBtn,[&](bool& stop)
         {
            sNum++;
            stop = true;
         });
         handler.add(m_lineUpBtn,[&](bool& stop)
         {
            // display the line-up cues
            m_newLineupDisplay.setFormatMode(2);
            m_newLineupDisplay.redraw("New line-up: ");
            m_newLineup.setFormatMode(2);
            m_newLineup.erase();
            for(size_t i=0;i<m_table.size();i++)
            {
               m_table[i].num.setFormatMode(2);
               std::stringstream s; s << i;
               m_table[i].num.redraw(s.str());
            }

            // solicit new line-up from user
            std::vector<size_t> newLineup;
            auto& keys = svcMan->demand<cui::iUserInput>();
            while(newLineup.size() != 4)
            {
               auto k = keys.getKey();
               if('0' <= k && k <= '4')
                  newLineup.push_back(k - '0');
               else
                  break;

               auto& pen = svcMan->demand<pen::object>();
               pen.str()
                  << pen::moveTo(cui::pnt(
                     m_newLineup.getLoc().x+newLineup.size()-1,
                     m_newLineup.getLoc().y))
                  << pen::bgcol(pen::kMagenta)
                  << std::string(1,k);
            }

            // remove the line-up cues
            m_newLineupDisplay.setFormatMode(1);
            m_newLineupDisplay.erase();
            m_newLineup.setFormatMode(1);
            m_newLineup.erase();
            for(size_t i=0;i<m_table.size();i++)
            {
               m_table[i].num.setFormatMode(1);
               m_table[i].num.erase();
            }

            if(newLineup.size() == 4)
            {
               // implement the new line-up
               sst::dict req;
               auto& noob = req.add<sst::array>("line-up");
               for(auto x : newLineup)
                  noob.append<sst::mint>() = x;
               ch.sendString("changeLineUp");
               ch.sendSst(req);
               acct.reset(ch.recvSst());

               stop = true; // only redraw if line-up changed
            }
         });
         handler.add(m_goBtn,[](bool& stop)
         {
            tcat::typePtr<cui::iFactory> sFac;
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("battle"));
            pL->run();
            stop = true;
         });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return; // TODO: useless
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("precombat") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
