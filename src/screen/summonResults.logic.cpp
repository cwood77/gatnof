#include "../../gen/screen/screen.summonResults.hpp"
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

class logic : private summonResults_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool)
   {
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& ch = svcMan->demand<net::iChannel>();
      std::unique_ptr<sst::dict> pReply(ch.recvSst());

      render();

      ::getch();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("summonResults") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
