#include "../../gen/screen/screen.charDetail.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include <memory>

namespace {

class logic : private charDetail_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      tcat::typePtr<db::iDict> dbDict;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();

      long pg = 0;

      // whole screen re-draw
      // note: this is here so I can read table size
      render();

      while(true)
      {
         // footer

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.addCustom('0',[&](bool& stop){  });
         handler.addCustom('1',[&](bool& stop){  });
         handler.addCustom('2',[&](bool& stop){  });
         handler.addCustom('3',[&](bool& stop){  });
         handler.addCustom('4',[&](bool& stop){  });
         handler.addCustom('5',[&](bool& stop){  });
         handler.addCustom('6',[&](bool& stop){  });
         handler.addCustom('7',[&](bool& stop){  });
         handler.addCustom('8',[&](bool& stop){  });
         handler.addCustom('9',[&](bool& stop){  });
         handler.addCustom('A',[&](bool& stop){  });
         handler.addCustom('B',[&](bool& stop){  });
         handler.addCustom('C',[&](bool& stop){  });
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
         handler.add(m_upBtn,[&](bool& stop){ pg--; stop = true; });
         handler.add(m_downBtn,[&](bool& stop){ pg++; stop = true; });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return;

         // whole screen re-draw
         render();
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("charDetail") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
