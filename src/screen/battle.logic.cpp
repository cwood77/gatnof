#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/ani.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>

namespace {

class logic : public cui::iLogic {
public:
   virtual void run(bool)
   {
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("battle_screen"));
      pScr->render();

      ::getch();

      // do an experiment
      auto& p1 = pScr->demand<cui::control>("p1");
      auto& o4 = pScr->demand<cui::control>("o4");

      ani::delay d;
      ani::flipbook fb(d);
      ani::sequencer seq(fb);

      ani::outliner artistp1(p1);
      ani::outliner artisto4(o4);
      seq.simultaneous(
      {
         [&](auto& c){ artistp1.outline(c); },
         [&](auto& c){ artisto4.outline(c); }
      });

      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& pn = svcMan->demand<pen::object>();
      fb.run(pn);

      /*p1.setFormatMode(2);
      p1.erase();*/

      ::getch();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("battle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
