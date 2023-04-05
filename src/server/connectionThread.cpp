#include "../console/log.hpp"
#include "connectionThread.hpp"
#include "msg.hpp"

namespace server {

void connectionThread::run()
{
   connectionContext ctxt;
   while(!ctxt.quit)
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
      log().writeLnVerbose("(%d) handling command '%s'",::GetCurrentThreadId(),cmd.c_str());

      auto& mh = msgRegistry::get().demand(cmd);
      tie(mh).run(*m_pChan,ctxt);
   }
}

} // namespace server
