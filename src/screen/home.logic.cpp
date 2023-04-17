#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../shell/gameState.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>

namespace {

class logic : public cui::iLogic {
public:
   virtual void run(bool)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      bool first = true;

      // fetch controls
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("home_screen"));
      auto& aName = pScr->demand<cui::stringControl>("aName");
      auto& gems = pScr->demand<cui::intControl>("gems");
      gems.userInitialize([&]()
      {
         gems.setFormatter(
            *new cui::hugeValueIntFormatter());
      });
      auto& gold = pScr->demand<cui::intControl>("gold");
      gold.userInitialize([&]()
      {
         gold.setFormatter(
            *new cui::hugeValueIntFormatter());
      });
      auto& inboxHint = pScr->demand<cui::intControl>("inboxHint");
      inboxHint.userInitialize([&]()
      {
         inboxHint.setFormatter(
            *new cui::bracketedIntFormatter(
               *new cui::maxValueIntFormatter()));
      });
      auto& error = pScr->demand<cui::stringControl>("error");
      auto& ip = pScr->demand<cui::stringControl>("ip");
      auto& year = pScr->demand<cui::stringControl>("year");

      // fetch buttons
      auto& inboxBtn = pScr->demand<cui::buttonControl>("inboxBtn");
      auto& summonBtn = pScr->demand<cui::buttonControl>("summonBtn");
      auto& charBtn = pScr->demand<cui::buttonControl>("charBtn");
      auto& questBtn = pScr->demand<cui::buttonControl>("questBtn");
      auto& arenaBtn = pScr->demand<cui::buttonControl>("arenaBtn");
      auto& exitBtn = pScr->demand<cui::buttonControl>("exitBtn");
      arenaBtn.dim("not yet implemented");

      while(true)
      {
         // allow fighting
         bool canFight = ((*acct)["line-up"].as<sst::array>().size() > 0);
         questBtn.dim("add at least one character to line-up first",!canFight);

         // whole screen re-draw
         pScr->render();

         // static controls
         aName.redraw(svcMan->demand<shell::gameState>().accountName);
         ip.redraw(svcMan->demand<shell::gameState>().serverIp);
         year.redraw("2023");

         // attendance
         if(first)
         {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("attend"));
            pL->run();
            first = false;
            continue;
         }

         // check for updates
         auto& ch = svcMan->demand<net::iChannel>();
         ch.sendString("update");
         acct.reset(ch.recvSst());

         // dynamic controls
         gems.redraw((*acct)["gems"].as<sst::mint>().get());
         gold.redraw((*acct)["gold"].as<sst::mint>().get());
         auto nInbox = (*acct)["inbox"].as<sst::array>().size();
         if(nInbox > 0)
            inboxHint.redraw(nInbox);

         // handle user input
         cui::buttonHandler handler(error);
         handler.add(inboxBtn,[&](bool& stop)
         {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("inbox"));
            pL->run();
            stop = true; // redraw home
         });
         handler.add(summonBtn,[&](bool& stop)
         {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("summon"));
            pL->run();
            stop = true; // redraw home
         });
         handler.add(charBtn,[&](bool& stop)
         {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("char"));
            pL->run();
            stop = true; // redraw home
         });
         handler.add(arenaBtn,[&](bool& stop)
         {
            {
            // TODO TEST!
            ch.sendString("queryCombat");
            {
               sst::dict req;
               req.add<sst::str>("type") = "quest";
               req.add<sst::mint>("quest#") = 1;
               req.add<sst::mint>("stage#") = 1;
               ch.sendSst(req);
            }
            std::unique_ptr<sst::dict> pCombatInfo(ch.recvSst());

            ch.sendString("combat");
            ch.sendSst(*pCombatInfo);
            delete ch.recvSst(); // throw it away
            delete ch.recvSst(); // throw it away
            }
            {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("battle"));
            pL->run();
            stop = true; // redraw home
            }
         });
         handler.add(questBtn,[&](bool& stop)
         {
            size_t qNum = 1;
            cmn::autoService<size_t> _qNumSvc(*svcMan,qNum,"selectedQuest");
            size_t sNum = 1;
            cmn::autoService<size_t> _sNumSvc(*svcMan,sNum,"selectedStage");

            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("precombat"));
            pL->run();

            stop = true; // redraw home
         });
         handler.add(exitBtn,[&](bool& stop){ stop = true; });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &exitBtn)
            return;
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("home") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
