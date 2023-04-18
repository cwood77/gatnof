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

      // query stage info
      std::unique_ptr<sst::dict> pCombatInfo;
      {
         bool unused = false;
         scrollQuest(ch,qNum,sNum,"=",pCombatInfo,unused);
      }
      while(true)
      {
         // footer
         m_error.update("");

         // header
         m_aName.redraw(svcMan->demand<shell::gameState>().accountName);
         m_qNum.redraw(qNum);
         m_sNum.redraw(sNum);
         updateForOldQuests(*acct,qNum,sNum);

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
            scrollQuest(ch,qNum,sNum,"--",pCombatInfo,stop);
         });
         handler.add(m_downBtn,[&](bool& stop)
         {
            scrollQuest(ch,qNum,sNum,"++",pCombatInfo,stop);
         });
         handler.add(m_lineUpBtn,[&](bool& stop)
         {
            handleLineUpReorder(acct,stop);
         });
         handler.add(m_goBtn,[&](bool& stop)
         {
            tcat::typePtr<cui::iFactory> sFac;
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("battle"));

            bool advQuest = false;
            cmn::autoService<bool> _advQuest(*svcMan,advQuest,"advQuest");

            pL->run();
            stop = true;

            render();
            if(advQuest)
               completeQuest(ch,qNum,sNum,pCombatInfo,acct);
            else
            {
               //bool unused = false;
               //scrollQuest(ch,qNum,sNum,"=",pCombatInfo,unused);
            }
         });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return; // TODO: useless
      }
   }

private:
   void updateForOldQuests(sst::dict& acct, size_t qNum, size_t sNum)
   {
      // show/hide past performance
      std::stringstream questMoniker;
      questMoniker << qNum << "-" << sNum;

      std::stringstream indicator;
      auto& qHistory = acct["quest-history"].as<sst::dict>();
      if(qHistory.has(questMoniker.str()))
      {
         auto& score = qHistory[questMoniker.str()].as<sst::dict>()["score"].as<sst::array>();
         indicator << "[completed ";
         for(size_t i=0;i<score.size();i++)
            indicator << (score[i].as<sst::tf>().get() ? "\xfb" : "x");
         indicator << "]";
      }
      m_completedInd.update(indicator.str());

      // dim/undim go button
      auto& cQuest = acct["current-quest"].as<sst::dict>();

      const bool isFutureQuest =
         cQuest["quest"].as<sst::mint>().get() < qNum ||
            (cQuest["quest"].as<sst::mint>().get() == qNum &&
             cQuest["stage"].as<sst::mint>().get() < sNum);

      m_goBtn.dim("you must complete quests in order",isFutureQuest);
      m_goBtn.redraw();
   }

   void handleLineUpReorder(std::unique_ptr<sst::dict>& acct, bool& stop)
   {
      size_t lineUpSize = (*acct)["line-up"].as<sst::array>().size();
      std::set<size_t> legalKeys;

      // display the line-up cues
      m_newLineupDisplay.setFormatMode(2);
      m_newLineupDisplay.redraw("New line-up: ");
      m_newLineup.setFormatMode(2);
      m_newLineup.erase();
      for(size_t i=0;i<lineUpSize;i++)
      {
         m_table[i].num.setFormatMode(2);
         std::stringstream s; s << i;
         m_table[i].num.redraw(s.str());
         legalKeys.insert(i);
      }

      // solicit new line-up from user
      std::vector<size_t> newLineup;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& ch = svcMan->demand<net::iChannel>();
      auto& keys = svcMan->demand<cui::iUserInput>();
      while(newLineup.size() < lineUpSize)
      {
         auto k = keys.getKey();
         if(legalKeys.find(k - '0') != legalKeys.end())
         {
            m_error.update("");

            newLineup.push_back(k - '0');
            legalKeys.erase(k - '0');
         }
         else if(k == 27) // escape
            break;
         else
         {
            m_error.redraw("illegal or duplicate entry");
            continue;
         }

         auto& pen = svcMan->demand<pen::object>();
         pen.str()
            << pen::moveTo(cui::pnt(
               m_newLineup.getLoc().x+newLineup.size()-1,
               m_newLineup.getLoc().y))
            << pen::fgcol(pen::kYellow)
            << pen::bgcol(pen::kMagenta)
            << std::string(1,k);
      }

      // remove the line-up cues
      m_newLineupDisplay.setFormatMode(1);
      m_newLineupDisplay.erase();
      m_newLineup.setFormatMode(1);
      m_newLineup.erase();
      for(size_t i=0;i<lineUpSize;i++)
      {
         m_table[i].num.setFormatMode(1);
         m_table[i].num.erase();
      }

      if(newLineup.size() == lineUpSize)
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
   }

   void scrollQuest(
      net::iChannel& ch,
      size_t& qNum,
      size_t& sNum,
      const std::string& mode,
      std::unique_ptr<sst::dict>& pCombatInfo,
      bool& stop)
   {
      ch.sendString("queryCombat");
      {
         sst::dict req;
         req.add<sst::str>("type") = "quest";
         req.add<sst::mint>("quest#") = qNum;
         req.add<sst::mint>("stage#") = sNum;
         req.add<sst::str>("mode") = mode;
         ch.sendSst(req);
      }
      std::string rsp = ch.recvString();
      if(!rsp.empty())
         m_error.redraw(rsp);
      else
      {
         pCombatInfo.reset(ch.recvSst());

         std::unique_ptr<sst::dict> pNewNums(ch.recvSst());
         qNum = (*pNewNums)["quest#"].as<sst::mint>().get();
         sNum = (*pNewNums)["stage#"].as<sst::mint>().get();

         stop = true;
      }
   }

   void completeQuest(
      net::iChannel& ch,
      size_t& qNum,
      size_t& sNum,
      std::unique_ptr<sst::dict>& pCombatInfo,
      std::unique_ptr<sst::dict>& acct)
   {
      ch.sendString("queryCombat");
      {
         sst::dict req;
         req.add<sst::str>("type") = "quest";
         req.add<sst::mint>("quest#") = qNum;
         req.add<sst::mint>("stage#") = sNum;
         req.add<sst::str>("mode") = "++!";
         ch.sendSst(req);
      }
      std::string rsp = ch.recvString();
      if(!rsp.empty())
         m_error.redraw(rsp);
      else
      {
         pCombatInfo.reset(ch.recvSst());
         acct.reset(ch.recvSst());
         qNum = (*acct)["current-quest"].as<sst::dict>()["quest"].as<sst::mint>().get();
         sNum = (*acct)["current-quest"].as<sst::dict>()["stage"].as<sst::mint>().get();
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("precombat") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
