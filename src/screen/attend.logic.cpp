#include "../../gen/screen/screen.attend.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/ani.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../file/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>

namespace {

class logic : private attend_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();

      // whole screen re-draw
      render();

      // static controls
      {
         ani::delay d;
         d.nMSec = 50;
         ani::flipbook fb(d);
         ani::sequencer seq(fb);

         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::colorBox(c,1); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::drawConnection(c,1); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::colorBox(c,2); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::drawConnection(c,2); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::colorBox(c,3); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::drawConnection(c,3); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::colorBox(c,4); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::drawConnection(c,4); }
         });
         seq.simultaneous(
         {
            [&](auto& c){ ani::attendance::colorBox(c,5); }
         });

         tcat::typePtr<cmn::serviceManager> svcMan;
         auto& pn = svcMan->demand<pen::object>();
         fb.run(pn);
      }

      // dynamic controls

      // handle user input
      svcMan->demand<cui::iUserInput>().getKey();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("attend") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
