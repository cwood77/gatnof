#include "../cmn/autoPtr.hpp"
#include "../cui/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>

namespace {

class logic : public cui::iLogic {
public:
   virtual void run()
   {
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("battle_screen"));
      pScr->render();

      ::getch();

      // do an experiment
      auto& p1 = pScr->demand<cui::control>("p1");
      p1.setFormatMode(2);
      p1.erase();

      ::getch();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("battle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
