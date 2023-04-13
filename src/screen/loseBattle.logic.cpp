#include "../../gen/screen/screen.loseBattle.hpp"
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

class logic : private loseBattle_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;

      // animation
      if(0) { // move to combat
         ani::delay d;
         d.nMSec = 1;
         d.nSkip = 2;
         ani::flipbook fb(d);
         ani::sequencer seq(fb);

         seq.simultaneous(
         {
            [&](auto& c){ ani::prim::lineLeftToRight(c,cui::pnt(1,8),110); },
            [&](auto& c){ ani::prim::lineRightToLeft(c,cui::pnt(1,14),110); }
            //[&](auto& c){ artistp1.outline(c); },
            //[&](auto& c){ artisto4.outline(c); }
         });

         tcat::typePtr<cmn::serviceManager> svcMan;
         auto& pn = svcMan->demand<pen::object>();
         fb.run(pn);
      }

      // whole screen re-draw
      render();

      // animate title
      m_title.update("F"); ::Sleep(250);
      m_title.update("FA"); ::Sleep(250);
      m_title.update("FAI"); ::Sleep(250);
      m_title.update("FAIL"); ::Sleep(250);
      m_title.update("FAILU"); ::Sleep(250);
      m_title.update("FAILUR"); ::Sleep(250);
      m_title.update("FAILURE"); ::Sleep(250);

      m_text0.update("Holly suggests:");
      m_text1.update("take a power nap and try again");

      // static controls
      if(interactive)
         m_prompt.redraw("<any key to dismiss>");
      else
      {
         m_prompt.setFormatMode(2);
         m_prompt.redraw("     -- auto --     ");
      }

      // handle user input
      svcMan->demand<cui::iUserInput>().getKey();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("loseBattle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
