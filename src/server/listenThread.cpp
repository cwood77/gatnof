#include "../cmn/autoPtr.hpp"
#include "listenThread.hpp"

namespace server {

void listenThread::run()
{
   while(true)
   {
      cmn::autoReleasePtr<net::iAllocChannel> pAChan(&m_netProto.createPeerChannelServer());
      cmn::autoReleasePtr<net::iChannel> pChan(&m_netProto.wrap(*pAChan.abdicate()));

      bool wasStopped = false;
      auto cmd = pChan->recvString(m_stopSignal,wasStopped);
      if(wasStopped)
         return; // shutting down
      else
      {
         auto& conTh = m_workers.allocate();
         tie(conTh);
         conTh.initialize(*pChan.abdicate(),m_stopSignal,cmd);
         m_workers.run(conTh);
      }
   }
}

} // namespace server
