#include "../console/log.hpp"
#include "channel.hpp"
#include <stdexcept>

namespace net {

std::string peerChannelBase::getPropStr(size_t key)
{
   throw std::runtime_error("property isn't supported by channel type");
}

void peerChannelBase::send(cmn::sizedAlloc& msg)
{
   m_sock.sendWithSize(msg);
}

void peerChannelBase::recv(cmn::sizedAlloc& msg)
{
   m_sock.recvWithSize(msg);
}

void peerChannelBase::waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped)
{
   autoSockEvent evt;
   evt.observeRecv(m_sock);
   evt.waitForever(stopSignal,wasStopped);
}

std::string peerChannelServer::getPropStr(size_t key)
{
   if(key == kConnectedIp)
      return m_connectedIp;
   else
      return peerChannelBase::getPropStr(key);
}

void peerChannelServer::send(cmn::sizedAlloc& msg)
{
   if(!m_isOpened)
      throw std::runtime_error("servers should recv(evt) to open");
   peerChannelBase::send(msg);
}

void peerChannelServer::recv(cmn::sizedAlloc& msg)
{
   if(!m_isOpened)
      throw std::runtime_error("servers should recv(evt) to open");
   peerChannelBase::recv(msg);
}

void peerChannelServer::waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped)
{
   if(!m_isOpened)
   {
      open(stopSignal,wasStopped);
      if(wasStopped)
         return;
   }
   peerChannelBase::waitForRecv(stopSignal,wasStopped);
}

// if server            if client (needs addr param)
// socket               socket
// bind
// listen               connect
// accept (2nd sock)
// recv                 send
//                      shutdown(send)
// send                 recv
// shutdown(send)
// close                close
void peerChannelServer::open(cmn::osEvent& stopSignal, bool& wasStopped)
{
   log().writeLnDebug("opening on first use");

   auto lsock = autoSocket::createForTcp();
   lsock.enableReuseAddr();

   sockaddr_in addr;
   sockHelper::ip4TcpSvr(addr);

   log().writeLnDebug("bind + listen + accept");
   lsock
      .bind(addr)
      .listen()
      .accept(stopSignal,m_sock,m_connectedIp,wasStopped);

   if(wasStopped)
      log().writeLnDebug("accept aborted");
   else
   {
      log().writeLnDebug("accept completed");
      m_isOpened = true;
   }
}

peerChannelClient& peerChannelClient::open(const std::string& ip)
{
   m_sock.setupForTcp();

   log().writeLnDebug("connect");

   std::vector<sockaddr_in> addrs;
   sockHelper::ip4ToTcpAddr(ip,"27015",addrs);
   for(auto addr : addrs)
   {
      log().writeLnTemp("  trying address <%s>",sockHelper::getIpAddr(addr).c_str());
      autoSockEvent evt;
      evt.observeConnect(m_sock);
      m_sock.beginConnect(addr);

      bool timedOut = false;
      log().writeLnTemp("    beginning wait...");
      evt.waitForSecs(3,timedOut);
      if(timedOut)
      {
         log().writeLnTemp("      timed out :(");
         throw std::runtime_error("timed out waiting for IP");
      }
      log().writeLnTemp("      success!!");

      return *this;
   }

   throw std::runtime_error("can't find any workable sockaddr_in for IP");
}

} // namespace net
