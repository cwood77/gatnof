#include "../cmn/autoPtr.hpp"
#include "../cmn/win32.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "broadcast.hpp"
#include "wsockhlp.hpp"
#include <stdexcept>

#include <ws2tcpip.h> // getaddrinfo et al.
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

namespace {

size_t getTimeoutInSecs(sst::dict& config)
{
   return config.getOpt<sst::mint>("network-timeout-in-seconds",5);
}

} // anonymous namespace

namespace net {

void udpSender::broadcastAndListen(const std::string& sendMessage, const std::string& expectedResponse, std::set<std::string>& responders)
{
   log().writeLnDebug("starting winsock");
   autoWinSockLibInit _winsock;

   log().writeLnDebug("setting up socket");
   auto sock = autoSocket::createForUdp();
   sock.enableBroadcast();

   log().writeLnDebug("confguing address");
   sockaddr_in dest;
   sockHelper::ip4Broadcast(dest);

   log().writeLnDebug("sendto");
   sock.sendTo(sendMessage,dest);

   autoSockEvent evt;
   evt.observeRecv(sock);

   while(true)
   {
      log().writeLnDebug("waiting for a response");
      bool timedOut;
      evt.waitForSecs(getTimeoutInSecs(config()),timedOut);
      if(timedOut)
      {
         log().writeLnDebug("giving up");
         break;
      }

      sockaddr_in responder;
      log().writeLnDebug("got one!");
      cmn::sizedAlloc block;
      block.realloc(4096);
      sock.recvFrom(block,responder);
      log().writeLnDebug("response from %s is %s",
         sockHelper::getIpAddr(responder).c_str(),
         block.ptr());

      if(::strcmp(expectedResponse.c_str(),block.ptr())==0)
         responders.insert(sockHelper::getIpAddr(responder));
      else
         log().writeLnDebug("VERY STRANGE: responder gave uexpected answer?");
   }
}

iResponse& udpListener::listen(cmn::osEvent& stopSignal, cmn::sizedAlloc& msg)
{
   sockaddr_in dest;
   sockHelper::ip4Any(dest);

   m_sock
      .setupForUdp()
      .enableBroadcast()
      .enableReuseAddr()
      .bind(dest);

   autoSockEvent evt;
   evt.observeRecv(m_sock);
   evt.waitForever(stopSignal,m_wasStopped);
   if(!m_wasStopped)
      m_sock.recvFrom(msg,m_addr);
   return *this;
}

void udpListener::respond(void *pBytes, size_t n)
{
   m_sock.sendTo((const char*)pBytes,n,m_addr); // TODO normalize these APIs
}

} // namespace net
