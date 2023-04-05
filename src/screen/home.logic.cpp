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
   virtual void run()
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      tcat::typePtr<cui::iFactory> sFac;

      while(true)
      {
         // check for updates
         auto& ch = svcMan->demand<net::iChannel>();
         ch.sendString("update");
         acct.reset(ch.recvSst());

         // --- draw
         cmn::autoReleasePtr<cui::iScreen> pScr(
            &sFac->create<cui::iScreen>("home_screen"));
         pScr->render();

         // initialize some controls
         auto& aName = pScr->demand<cui::stringControl>("aName");
         aName.update(svcMan->demand<shell::gameState>().accountName);

         auto& gems = pScr->demand<cui::stringControl>("gems");
         gems.update((*acct)["gems"].as<sst::str>().get());

         auto& gold = pScr->demand<cui::stringControl>("gold");
         gold.update((*acct)["gold"].as<sst::str>().get());

         auto& inboxHint = pScr->demand<cui::intControl>("inboxHint");
         inboxHint.userInitialize([&]()
         {
            inboxHint.setFormatter(
               *new cui::bracketedIntFormatter(
                  *new cui::maxValueIntFormatter()));
         });
         auto nInbox = (*acct)["inbox"].as<sst::array>().size();
         inboxHint.setFormatMode(nInbox > 0 ? 2 : 1);
         inboxHint.update(nInbox);

         auto& ip = pScr->demand<cui::stringControl>("ip");
         ip.update(svcMan->demand<shell::gameState>().serverIp);

         auto& year = pScr->demand<cui::stringControl>("year");
         year.update("2023");

         auto& error = pScr->demand<cui::stringControl>("error");

         // wait for keyboard input
         bool done = false;
         while(!done)
         {
            char c = ::getch();
            switch(c)
            {
               case 'i':
               case 's':
               case 'c':
               case 'l':
               case 'q':
                  error.update("Unimpled");
                  break;
               case 'a':
                  {
                     cmn::autoReleasePtr<cui::iLogic> pL(
                        &sFac->create<cui::iLogic>("battle"));
                     pL->run();
                     done = true;
                  }
                  break;
               case 'e':
                  return;
               default:
                  error.update("Unrecognized command");
                  break;
            }
         }
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("home") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
