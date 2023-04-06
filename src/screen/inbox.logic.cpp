#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../file/api.hpp"
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
      //auto& table = pScr->demand<cui::listControl>("table");
      auto& error = pScr->demand<cui::stringControl>("error");

      // fetch buttons
      auto& backBtn = pScr->demand<cui::buttonControl>("backBtn");
      auto& openNextBtn = pScr->demand<cui::buttonControl>("openNextBtn");
      openNextBtn.dim("not yet implemented");

      while(true)
      {
         // whole screen re-draw
         pScr->render();

         // static controls

         // dynamic controls
         auto nInbox = (*acct)["inbox"].as<sst::array>().size();
         count.redraw(nInbox);

         // handle user input
         cui::buttonHandler handler(error);
         handler.add(backBtn,[&](auto& bnt, bool& stop){ stop = true; });
         handler.add(openNextBtn,[&](auto& bnt, bool&){ });
         auto& ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(&ans == &backBtn)
            return;
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("inbox") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
