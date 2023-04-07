#include "../../gen/screen/screen.get.hpp"
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

class logic : private get_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      auto& prize = (*acct)["inbox"].as<sst::array>()[0].as<sst::dict>();

      // animation
      if(0){ // too slow, but keep for attendance?
         ani::delay d;
         d.nMSec = 1;
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

      // static controls
      if(interactive)
         m_prompt.redraw("<any key to dismiss>");
      else
      {
         m_prompt.setFormatMode(2);
         m_prompt.redraw("     -- auto --     ");
      }

      // dynamic controls
      m_msg.redraw(prize["reason"].as<sst::str>().get());
      {
         std::stringstream stream;
         stream << prize["amt"].as<sst::mint>().get() << " ";
         stream << prize["unit"].as<sst::str>().get();
         m_booty.redraw(stream.str());
      }

      // handle user input
      svcMan->demand<cui::iUserInput>().getKey();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("get") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
