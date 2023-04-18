#include "../../gen/screen/screen.inbox.hpp"
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

class logic : public cui::iLogic {
public:
   virtual void run(bool)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();

      // fetch controls
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("inbox_screen"));
      auto& count = pScr->demand<cui::intControl>("count");
      count.userInitialize([&]()
      {
         count.setFormatter(
            *new cui::bracketedIntFormatter(
               *new cui::maxValueIntFormatter()));
      });
      auto& table = pScr->demand<cui::listControl<inbox_table_row_ctl> >("table");
      auto& error = pScr->demand<cui::stringControl>("error");

      // fetch buttons
      auto& backBtn = pScr->demand<cui::buttonControl>("backBtn");
      auto& openNextBtn = pScr->demand<cui::buttonControl>("openNextBtn");

      while(true)
      {
         // whole screen re-draw
         pScr->render();

         // static controls

         // dynamic controls
         auto& inboxData = (*acct)["inbox"].as<sst::array>();
         count.redraw(inboxData.size());
         for(size_t i=0;i<inboxData.size();i++)
         {
            if(i >= table.size())
               break; // out of rows for now

            auto& elt = table[i];
            fmtPrize(inboxData[i].as<sst::dict>(),elt);
         }
         if(inboxData.size() == 0)
         {
            openNextBtn.dim("inbox is empty");
            openNextBtn.redraw();
         }

         // handle user input
         cui::buttonHandler handler(error);
         handler.add(backBtn,[&](bool& stop){ stop = true; });
         handler.add(openNextBtn,[&](bool& stop)
         {
            cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("get"));
            pL->run();

            auto& ch = svcMan->demand<net::iChannel>();
            ch.sendString("get");
            acct.reset(ch.recvSst());

            stop = true;
         });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &backBtn)
            return;
      }
   }

   virtual void fmtPrize(sst::dict& d, inbox_table_row_ctl& row)
   {
      std::stringstream reason,amt;

      {
         time_t ts = (time_t)d["when"].as<sst::mint>().get();
         struct tm *pLt = ::localtime(&ts);
         char block[1024];
         ::strftime(block,1023,"%m/%d/%Y %H:%M ",pLt);
         reason << block << d["reason"].as<sst::str>().get();
      }

      amt << d["amt"].as<sst::mint>().get() << " " << d["unit"].as<sst::str>().get();

      row.date.redraw(reason.str());
      row.prize.redraw(amt.str());
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("inbox") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
