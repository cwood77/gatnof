#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>

namespace {

class logic : public cui::iLogic {
public:
   virtual void run(bool)
   {
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iScreen> pScr(&sFac->create<cui::iScreen>("inbox_screen"));
      pScr->render();

      ::getch();
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("inbox") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
