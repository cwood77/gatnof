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
      auto& aName = pScr->demand<cui::control>("aName");
      aName.access([](auto& s){ s = "EthanTheStinky"; });

      auto& gems = pScr->demand<cui::control>("gems");
      gems.access([](auto& s){ s = "0"; });

      auto& gold = pScr->demand<cui::control>("gold");
      gold.access([](auto& s){ s = "0"; });

      auto& ip = pScr->demand<cui::control>("ip");
      ip.access([](auto& s){ s = "123.123.123.123"; });

      auto& year = pScr->demand<cui::control>("year");
      year.access([](auto& s){ s = "2023"; });

      auto& error = pScr->demand<cui::control>("error");

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
               error.access([](auto& s){ s="Unimpled"; });
               break;
            case 'e':
               return;
            default:
               error.access([](auto& s){ s="Unrecognized command"; });
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
