#ifndef ___server_awardThread___
#define ___server_awardThread___

#include "../cmn/subobject.hpp"
#include "../cmn/win32.hpp"

namespace server {

class awardThread : public cmn::iThread, public cmn::subobject {
public:
   awardThread(cmn::osEvent& stopSignal) : m_stopSignal(stopSignal) {}

   virtual void run();

private:
   cmn::osEvent& m_stopSignal;
};

} // namespace server

#endif // ___server_awardThread___
