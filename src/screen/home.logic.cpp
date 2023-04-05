#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../file/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>

namespace {

class logic : public cui::iLogic {
public:
   virtual void run()
   {
      while(true)
      {
         tcat::typePtr<cmn::serviceManager> svcMan;
         auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();

         tcat::typePtr<cui::iFactory> sFac;
         cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("home_screen"));
         pScr->render();

         // initialize some controls
         auto& aName = pScr->demand<cui::stringControl>("aName");
         aName.update((*acct)["accountName"].as<sst::str>().get());

         auto& gems = pScr->demand<cui::stringControl>("gems");
         gems.update((*acct)["gems"].as<sst::str>().get());

         auto& gold = pScr->demand<cui::stringControl>("gold");
         gold.update((*acct)["gold"].as<sst::str>().get());

         auto& ip = pScr->demand<cui::stringControl>("ip");
         ip.update((*acct)["server-ip"].as<sst::str>().get());

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
                     cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("battle"));
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
