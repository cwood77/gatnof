#include "../cmn/win32.hpp"
#include "wsockhlp.hpp"
#include <sstream>
#include <stdexcept>
#include <ws2tcpip.h>

namespace net {

autoWinSockLibInit::autoWinSockLibInit()
{
   ::WSAStartup(MAKEWORD(2,2),&m_data);
}

autoWinSockLibInit::~autoWinSockLibInit()
{
   ::WSACleanup();
}

void sockHelper::enableBroadcast(SOCKET& s)
{
   BOOL v = TRUE;
   auto rval = ::setsockopt(s,SOL_SOCKET,SO_BROADCAST,(const char *)&v,sizeof(BOOL));
   if(rval < 0) // TODO is this right?
      throwSockEx("setsockopt");
}

void sockHelper::enableReuseAddr(SOCKET& s)
{
   BOOL v = TRUE;
   auto rval = ::setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(const char *)&v,sizeof(BOOL));
   if(rval < 0) // TODO is this right?
      throwSockEx("setsockopt");
}

void sockHelper::ip4Broadcast(sockaddr_in& a)
{
   a.sin_family       = AF_INET;
   a.sin_port         = htons(9009); // TODO hardcoded!
   a.sin_addr.s_addr  = INADDR_BROADCAST;
}

void sockHelper::ip4Any(sockaddr_in& a)
{
   a.sin_family       = AF_INET;
   a.sin_port         = htons(9009); // TODO hardcoded!
   a.sin_addr.s_addr  = INADDR_ANY;
}

void sockHelper::ip4TcpSvr(sockaddr_in& a)
{
   a.sin_family       = AF_INET;
   //a.sin_port         = 34665;//htons(9009); // TODO hardcoded!
   a.sin_port         = htons(27015); // TODO hardcoded!
   a.sin_addr.s_addr  = INADDR_ANY;
}

void sockHelper::ip4ToTcpAddr(const std::string& ip, const std::string& port, std::vector<sockaddr_in>& addr)
{
   addrinfo hints;
   ::memset(&hints,0,sizeof(addrinfo));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   addrinfo *result = NULL;
   auto rval = ::getaddrinfo(ip.c_str(),port.c_str(),&hints,&result);
   if(rval != 0)
      sockHelper::throwSockEx("getaddrinfo");

   auto *ptr = result;
   for(;ptr;ptr=ptr->ai_next)
   {
      if(ptr->ai_addrlen < sizeof(sockaddr_in))
         continue;
      addr.push_back(*(sockaddr_in*)ptr->ai_addr);
   }

   ::freeaddrinfo(result);
}

std::string sockHelper::getIpAddr(sockaddr_in& a)
{
   return inet_ntoa(a.sin_addr);
}

void sockHelper::throwSockEx(const std::string& func)
{
   auto x = ::WSAGetLastError();
   throwSockEx(x,func);
}

void sockHelper::throwSockEx(int error, const std::string& func)
{
   std::stringstream message;
   message << "Winsock error " << error << " calling function '" << func << "'";
   throw std::runtime_error(message.str());
}

autoSocket autoSocket::createForUdp()
{
   autoSocket self;
   self.setupForUdp();
   return self;
}

autoSocket autoSocket::createForTcp()
{
   autoSocket self;
   self.setupForTcp();
   return self;
}

autoSocket::~autoSocket()
{
   ::closesocket(m_sock);
}

autoSocket& autoSocket::setupForUdp()
{
   m_sock = ::socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
   return *this;
}

autoSocket& autoSocket::setupForTcp()
{
   m_sock = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
   return *this;
}

autoSocket& autoSocket::enableBroadcast()
{
   sockHelper::enableBroadcast(m_sock);
   return *this;
}

autoSocket& autoSocket::enableReuseAddr()
{
   sockHelper::enableReuseAddr(m_sock);
   return *this;
}

autoSocket& autoSocket::bind(sockaddr *pAddr, size_t addrLen)
{
   auto rval = ::bind(m_sock,pAddr,addrLen);
   if(rval < 0) // TODO is this right?
      sockHelper::throwSockEx("bind");
   return *this;
}

autoSocket& autoSocket::sendTo(const char *pMsg, size_t msgLen, sockaddr *pAddr, size_t addrLen)
{
   ::sendto(m_sock,pMsg,msgLen,0,pAddr,addrLen); // TODO why 0?
   return *this;
}

autoSocket& autoSocket::recvFrom(char *pMsg, size_t msgLen, sockaddr *pAddr, size_t addrLen)
{
   int addrLen2 = addrLen; // TODO why?
   ::recvfrom(m_sock,pMsg,msgLen,0,pAddr,&addrLen2);
   return *this;
}

autoSocket& autoSocket::listen()
{
   auto result = ::listen(m_sock,SOMAXCONN);
   if(result == SOCKET_ERROR)
      sockHelper::throwSockEx("listen");
   return *this;
}

autoSocket& autoSocket::accept(cmn::osEvent& stopSignal, autoSocket& Csock, std::string& connectedIp, bool& wasStopped)
{
   autoSockEvent wsEvt;
   wsEvt.observeAccept(*this);
   wsEvt.waitForever(stopSignal,wasStopped);
   if(wasStopped)
      return *this;

   int addrSize = sizeof(sockaddr_in);
   sockaddr_in client;
   Csock.handle() = ::accept(m_sock,(sockaddr*)&client,&addrSize);
   if(((size_t)addrSize) < sizeof(sockaddr_in))
      sockHelper::throwSockEx("accept");
   connectedIp = sockHelper::getIpAddr(client);

   return *this;
}

autoSocket& autoSocket::beginConnect(sockaddr *pAddr, size_t addrLen)
{
   auto iResult = ::connect(m_sock,pAddr,addrLen);
   if(iResult == SOCKET_ERROR)
   {
      auto x = ::WSAGetLastError();
      if(x != WSAEWOULDBLOCK)
         sockHelper::throwSockEx("connect");
   }
   return *this;
}

#if 0
autoSocket& autoSocket::connectTcp(const std::string& ip, const std::string& port)
{
   addrinfo hints;
   ::memset(&hints,0,sizeof(addrinfo));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

   addrinfo *result = NULL;
   auto rval = ::getaddrinfo(ip.c_str(),port.c_str(),&hints,&result);
   if(rval != 0)
      sockHelper::throwSockEx("getaddrinfo");
   auto *ptr = result;
   for(;ptr;ptr=ptr->ai_next)
   {
#if 0
::printf("Z%lld, <%hd,%hu,%d%d%d%d%d%d%d%d>, <%ld>\r\n",
      (size_t)ptr->ai_addrlen,
      ((sockaddr_in*)ptr->ai_addr)->sin_family,
      ((sockaddr_in*)ptr->ai_addr)->sin_port,
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[0],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[1],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[2],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[3],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[4],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[5],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[6],
      (int)((sockaddr_in*)ptr->ai_addr)->sin_zero[7],
      ((sockaddr_in*)ptr->ai_addr)->sin_addr.s_addr
);
#endif
      //if(ptr->ai_addrlen < sizeof(sockaddr_in))
      //   continue;
      //::memcpy(&a,ptr->ai_addr,sizeof(sockaddr_in));

::printf("setting up event\r\n");
      autoSockEvent evt;
      evt.observeConnect(*this);
::printf("attempting connect\r\n");
      auto iResult = ::connect(m_sock,ptr->ai_addr,ptr->ai_addrlen);
::printf("connect returned %d\r\n",iResult);
      if(iResult == SOCKET_ERROR)
      {
         auto x = ::WSAGetLastError();
         if(x != WSAEWOULDBLOCK)
         {
            ::printf("don't understand rval from connect %d\r\n",x);
            sockHelper::throwSockEx("connect");
         }
      }
      iResult = 0;

::printf("waiting on event\r\n");
      bool timedOut = false;
      evt.waitForSecs(3,timedOut);
::printf("   => %s timeout\r\n",timedOut ? "DID" : "did not");

      if(iResult == SOCKET_ERROR || timedOut)
      {
::printf("throwing from bad error\r\n");
            sockHelper::throwSockEx("connect");
         if(ptr->ai_next)
            continue;
         else
            sockHelper::throwSockEx("connect");
      }

#if 0
::printf("moving to blocking mode\r\n");
      {
         u_long iMode = 0;
         auto iResult = ioctlsocket(m_sock, FIONBIO, &iMode);
         if (iResult != NO_ERROR)
            ::printf("ioctlsocket failed with error: %d\n", iResult);
      }
#endif

      ::freeaddrinfo(result);
      return *this;
   }

   throw std::runtime_error("can't find valid address in connectTcp");
}
#endif

autoSocket& autoSocket::sendWithSize(cmn::sizedAlloc& msg)
{
   size_t size = msg.size();
   _sendWithSize((char*)&size,sizeof(size_t));

   _sendWithSize(msg.ptr(),size);

   return *this;
}

autoSocket& autoSocket::recvWithSize(cmn::sizedAlloc& msg)
{
   size_t size = 0;
   _recvWithSize((char*)&size,sizeof(size_t));

   msg.realloc(size);
   _recvWithSize(msg.ptr(),size);

   return *this;
}

autoSocket& autoSocket::_sendWithSize(char *pPtr, int n)
{
   while(n)
   {
      auto rval = ::send(m_sock,pPtr,n,0);
      if(rval == SOCKET_ERROR)
      {
         auto x = ::WSAGetLastError();
         if(x != WSAEWOULDBLOCK)
            sockHelper::throwSockEx(x,"send");
      }
      else
      {
         pPtr += rval;
         n -= rval;
      }
   }

   return *this;
}

autoSocket& autoSocket::_recvWithSize(char *pPtr, int n)
{
   while(n)
   {
      auto rval = ::recv(m_sock,pPtr,n,0);
      if(rval == SOCKET_ERROR)
      {
         auto x = ::WSAGetLastError();
         if(x != WSAEWOULDBLOCK)
            sockHelper::throwSockEx(x,"recv");
      }
      else
      {
         pPtr += rval;
         n -= rval;
      }
   }

   return *this;
}

autoSockEvent::autoSockEvent()
{
   m_hEvt = ::WSACreateEvent();
   if(m_hEvt == WSA_INVALID_EVENT)
      sockHelper::throwSockEx("WSACreateEvent");
}

autoSockEvent::~autoSockEvent()
{
   ::WSACloseEvent(m_hEvt);
}

autoSockEvent& autoSockEvent::observeRecv(autoSocket& sock)
{
   auto rVal = ::WSAEventSelect(sock.handle(),m_hEvt,FD_READ);
   if(rVal == SOCKET_ERROR)
      sockHelper::throwSockEx("WSAEventSelect");
   m_pSock = &sock;
   return *this;
}

autoSockEvent& autoSockEvent::observeAccept(autoSocket& sock)
{
   auto rVal = ::WSAEventSelect(sock.handle(),m_hEvt,FD_ACCEPT);
   if(rVal == SOCKET_ERROR)
      sockHelper::throwSockEx("WSAEventSelect");
   m_pSock = &sock;
   return *this;
}

autoSockEvent& autoSockEvent::observeConnect(autoSocket& sock)
{
   auto rVal = ::WSAEventSelect(sock.handle(),m_hEvt,FD_CONNECT);
   if(rVal == SOCKET_ERROR)
      sockHelper::throwSockEx("WSAEventSelect");
   m_pSock = &sock;
   return *this;
}

void autoSockEvent::waitForSecs(size_t secs, bool& timedOut)
{
   DWORD timeOutInMilliSecs = secs * 1000;
   DWORD rVal = ::WaitForSingleObject(m_hEvt,timeOutInMilliSecs);

   timedOut = (rVal == WAIT_TIMEOUT);

   if(rVal == WAIT_OBJECT_0)
   {
      // necessary to reset the event; even though I don't need the output
      WSANETWORKEVENTS info;
      auto error = ::WSAEnumNetworkEvents(m_pSock->handle(),m_hEvt,&info);
      if(error == SOCKET_ERROR)
         sockHelper::throwSockEx("WSAEnumNetworkEvents");
   }
}

void autoSockEvent::waitForever(cmn::osEvent& stopSignal, bool& wasStopped)
{
   HANDLE hans[2] = { stopSignal.getHandle(), m_hEvt };
   DWORD rVal = ::WaitForMultipleObjects(
      2,
      hans,
      FALSE, // any is fine
      INFINITE);

   wasStopped = (rVal == WAIT_OBJECT_0);

   if(rVal == (WAIT_OBJECT_0 + 1))
   {
      // necessary to reset the event; even though I don't need the output
      WSANETWORKEVENTS info;
      auto error = ::WSAEnumNetworkEvents(m_pSock->handle(),m_hEvt,&info);
      if(error == SOCKET_ERROR)
         sockHelper::throwSockEx("WSAEnumNetworkEvents");
   }
}

} // namespace net
