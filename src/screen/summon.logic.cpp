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
      auto& ch = svcMan->demand<net::iChannel>();

      ch.sendString("querySummon");
      std::unique_ptr<sst::dict> pSummonInfo(ch.recvSst());

      size_t upDown = 0;
      size_t leftRight = 0;

      while(true)
      {
         // whole screen re-draw
         render();

         // static controls - up/down
         {
            m_norm.setFormatMode(upDown == 0 ? 2 : 1);
            m_norm.erase();
            auto& pt = m_norm.getLoc();
            m_norm.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "Normal Summon";
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+2));
            pn.str() << (*pSummonInfo)["normal-info"].as<sst::str>().get();
         }
         {
            m_ur.setFormatMode(upDown == 1 ? 2 : 1);
            m_ur.erase();
            auto& pt = m_ur.getLoc();
            m_ur.formatText(pn.str());
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+1));
            pn.str() << "Ultra Rare Summon";
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+2));
            pn.str() << (*pSummonInfo)["ur-info"].as<sst::str>().get();
         }

         // static controls - left/right
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
            pn.str() << pen::moveTo(cui::pnt(pt.x+1,pt.y+2));
            pn.str()
               << (*pSummonInfo)["max-buy"].as<sst::array>()[upDown].as<sst::mint>().get();
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
         handler.add(m_summonBtn,[&](bool& stop)
         {
            ch.sendString("summon");
            sst::dict args;
            args.add<sst::mint>("upDown") = upDown;
            args.add<sst::mint>("leftRight") = leftRight;
            ch.sendSst(args);
            ch.sendSst(*pSummonInfo);
            std::unique_ptr<sst::dict> pReply(ch.recvSst());
            acct.reset(ch.recvSst());
            if(pReply->has("error"))
               m_error.redraw((*pReply)["error"].as<sst::str>().get());
            else
            {
               // unimpled!
               stop = true;
            }
         });
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
