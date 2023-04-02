#include "../cmn/autoPtr.hpp"
#include "../cui/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>

class logic : public cui::iLogic {
public:
   virtual void run()
   {
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("home_screen"));
      pScr->render();

      // initialize some controls
      auto& aName = pScr->demand<cui::stringControl>("aName");
      aName.update("EthanTheStinky");

      auto& gems = pScr->demand<cui::stringControl>("gems");
      gems.update("0");

      auto& gold = pScr->demand<cui::stringControl>("gold");
      gold.update("0");

      auto& ip = pScr->demand<cui::stringControl>("ip");
      ip.update("123.123.123.123");

      auto& year = pScr->demand<cui::stringControl>("year");
      year.update("2023");

      auto& error = pScr->demand<cui::stringControl>("error");

      // wait for keyboard input
      while(true)
      {
         char c = ::getch();
         switch(c)
         {
            case 'i':
            case 's':
            case 'c':
            case 'l':
            case 'q':
            case 'a':
               error.update("Unimpled");
               break;
            case 'e':
               return;
            default:
               error.update("Unrecognized command");
               break;
         }
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("home") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);
