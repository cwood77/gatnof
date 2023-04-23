#include "../../gen/screen/screen.winBattle.hpp"
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

class logic : private winBattle_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& battleDetails = svcMan->demand<sst::dict>("battleDetails");

      // animation
      { // move to combat
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

      // dynamic controls
      {
         auto& awards = battleDetails["awards"].as<sst::array>();
         for(size_t i=0;i<awards.size();i++)
         {
            auto& award = awards[i].as<sst::dict>();
            auto boonFullName = award["boon"].as<sst::str>().get();
            if(award["alreadyHad"].as<sst::tf>().get())
               boonFullName += " [already received]";
            m_table[i].condition.update(award["condition"].as<sst::str>().get());
            m_table[i].boon.update(boonFullName);
         }
      }

      // animate boons
      {
         auto& awards = battleDetails["awards"].as<sst::array>();
         for(size_t i=0;i<awards.size();i++)
         {
            ::Sleep(1000);

            auto& award = awards[i].as<sst::dict>();
            bool earned = award["earned"].as<sst::tf>().get();

            m_table[i].glyph.redraw(earned ? "\xfb" : "x");
            if(earned)
            {
               m_table[i].condition.setFormatMode(2);
               m_table[i].condition.redraw(award["condition"].as<sst::str>().get());

               bool already = award["alreadyHad"].as<sst::tf>().get();
               if(!already)
               {
                  m_table[i].boon.setFormatMode(2);
                  m_table[i].boon.redraw(award["boon"].as<sst::str>().get());
               }
            }
         }
      }

      // static controls
      ::Sleep(1000);
      if(interactive)
         m_prompt.redraw("<any key to dismiss>");
      else
      {
         m_prompt.setFormatMode(2);
         m_prompt.redraw("     -- auto --     ");
      }

      // handle user input
      auto& in = svcMan->demand<cui::iUserInput>();
      in.flush();
      in.getKey();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("winBattle") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
