#ifndef ___server_listenThread___
#define ___server_listenThread___

#include "../cmn/subobject.hpp"
#include "../cmn/win32.hpp"
#include "../net/api.hpp"
#include "connectionThread.hpp"

namespace server {

class listenThread : public cmn::iThread, public cmn::subobject {
public:
   listenThread(
      net::iNetProto& n,
      cmn::osEvent& stopSignal,
      cmn::threadGroup<connectionThread>& workers)
   : m_netProto(n), m_stopSignal(stopSignal), m_workers(workers) {  }

   virtual void run();

private:
   net::iNetProto& m_netProto;
   cmn::osEvent& m_stopSignal;
   cmn::threadGroup<connectionThread>& m_workers;
};

} // namespace server

#endif // ___server_listenThread___
