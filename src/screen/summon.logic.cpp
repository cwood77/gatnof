#include "../../gen/screen/screen.summon.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>

namespace {

class logic : private summon_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      auto& pn = svcMan->demand<pen::object>();

      sst::dict summonInfo;
      {
         summonInfo.add<sst::str>("normal-info") = "10 gems: 70% R, 20% SR, 10% SSR";
         summonInfo.add<sst::str>("ur-info") = "100 gems: 77% SR, 20% SSR, 3% UR";
         summonInfo.add<sst::mint>("max-buy") = 132;
      }

      size_t upDown = 0;
      size_t leftRight = 0;

      while(true)
      {
         // whole screen re-draw
         render();

         // static controls
         {
            m_norm.setFormatMode(upDown == 0 ? 2 : 1);
            m_norm.erase();
            auto& pt = m_norm.getLoc();
            m_norm.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "Normal Summon";
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+2));
            pn.str() << summonInfo["normal-info"].as<sst::str>().get();
         }
         {
            m_ur.setFormatMode(upDown == 1 ? 2 : 1);
            m_ur.erase();
            auto& pt = m_ur.getLoc();
            m_ur.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "Ultra Rare Summon";
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+2));
            pn.str() << summonInfo["ur-info"].as<sst::str>().get();
         }

         {
            m_draw1.setFormatMode(leftRight == 0 ? 2 : 1);
            m_draw1.erase();
            auto& pt = m_draw1.getLoc();
            m_draw1.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "  Draw 1";
         }
         {
            m_draw10.setFormatMode(leftRight == 1 ? 2 : 1);
            m_draw10.erase();
            auto& pt = m_draw10.getLoc();
            m_draw10.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << " Draw 10";
         }
         {
            m_drawMax.setFormatMode(leftRight == 2 ? 2 : 1);
            m_drawMax.erase();
            auto& pt = m_drawMax.getLoc();
            m_drawMax.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "Draw Max";
         }

         // dynamic controls

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
         handler.add(m_chgSumBtn,[&](bool& stop)
         {
            upDown++;
            upDown %= 2;
            stop = true;
         });
         handler.add(m_chgAmtBtn,[&](bool& stop)
         {
            leftRight++;
            leftRight %= 3;
            stop = true;
         });
         handler.unimpled(m_summonBtn);
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return;
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("summon") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
