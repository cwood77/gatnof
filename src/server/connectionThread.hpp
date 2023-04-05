#ifndef ___server_connectionThread___
#define ___server_connectionThread___

#include "../cmn/win32.hpp"
#include "../net/api.hpp"

namespace server {

class connectionThread : public cmn::iThread, public cmn::subobject {
public:
   void initialize(net::iChannel& c, cmn::osEvent& stopSignal, const std::string& cmd)
   { m_pChan = &c; m_pStopSignal = &stopSignal; m_cmd = cmd; }

   virtual void run();

private:
   net::iChannel *m_pChan;
   cmn::osEvent *m_pStopSignal;
   std::string m_cmd;
};

} // namespace server

#endif // ___server_connectionThread___
