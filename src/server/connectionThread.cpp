#include "connectionThread.hpp"

namespace server {

void connectionThread::run()
{
   while(true)
   {
      std::string cmd = m_cmd;
      m_cmd = "";
      if(cmd.empty())
      {
         bool wasStopped = false;
         cmd = m_pChan->recvString(*m_pStopSignal,wasStopped);
         if(wasStopped)
            return;
      }

      m_pChan->sendString("awk");
   }
}

} // namespace server
