#ifndef ___net_wsockhlp___
#define ___net_wsockhlp___

#include "../cmn/autoPtr.hpp"

namespace cmn { class osEvent; }

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <vector>
#include <windows.h>
#include <winsock2.h>

namespace net {

class autoWinSockLibInit {
public:
   autoWinSockLibInit();
   ~autoWinSockLibInit();

private:
   WSADATA m_data;
};

class sockHelper {
public:
   static void enableBroadcast(SOCKET& s);
   static void enableReuseAddr(SOCKET& s);

   static void ip4Broadcast(sockaddr_in& a);
   static void ip4Any(sockaddr_in& a);
   static void ip4TcpSvr(sockaddr_in& a);

   static void ip4ToTcpAddr(
      const std::string& ip, const std::string& port, std::vector<sockaddr_in>& addr);
   static std::string getIpAddr(sockaddr_in& a);

   static void throwSockEx(const std::string& func);
   static void throwSockEx(int error, const std::string& func);
};

class autoSocket {
public:
   static autoSocket createForUdp();
   static autoSocket createForTcp();
   ~autoSocket();

   autoSocket& setupForUdp();
   autoSocket& setupForTcp();
   autoSocket& enableBroadcast();
   autoSocket& enableReuseAddr();

   autoSocket& bind(sockaddr *pAddr, size_t addrLen);
   template<class A> autoSocket& bind(const A& addr)
   {
      return bind(
         const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&addr)),sizeof(A));
   }

   autoSocket& sendTo(const char *pMsg, size_t msgLen, sockaddr *pAddr, size_t addrLen);
   template<class A>
   autoSocket& sendTo(const char *pMsg, size_t msgLen, const A& addr)
   {
      return sendTo(
         pMsg,msgLen,
         const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&addr)),sizeof(A));
   }
   template<class A>
   autoSocket& sendTo(cmn::sizedAlloc& msg, A& addr)
   {
      return sendTo(
         msg.ptr(),msg.size(),
         const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&addr)),sizeof(A));
   }
   template<class A>
   autoSocket& sendTo(const std::string& msg, const A& addr)
   { return sendTo(msg.c_str(),msg.length()+1,addr); }

   autoSocket& recvFrom(char *pMsg, size_t msgLen, sockaddr *pAddr, size_t addrLen);
   template<class A>
   autoSocket& recvFrom(char *pMsg, size_t msgLen, A& addr);
   template<class A>
   autoSocket& recvFrom(cmn::sizedAlloc& msg, A& addr)
   {
      return recvFrom(
         msg.ptr(),msg.size(),
         const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&addr)),sizeof(A));
   }

   autoSocket& listen();

   autoSocket& accept(cmn::osEvent& stopSignal, autoSocket& Csock,
      std::string& connectedIp, bool& wasStopped);

   autoSocket& beginConnect(sockaddr *pAddr, size_t addrLen);
   template<class A>
   autoSocket& beginConnect(A& addr)
   {
      return beginConnect(
         const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&addr)),
         sizeof(A));
   }
   //autoSocket& connectTcp(const std::string& ip, const std::string& port);

   autoSocket& send(cmn::sizedAlloc& msg)
   { sockaddr_in unused; return sendTo(msg,unused); }
   autoSocket& recv(cmn::sizedAlloc& msg)
   { sockaddr_in unused; return recvFrom(msg,unused); }

   autoSocket& sendWithSize(cmn::sizedAlloc& msg);
   autoSocket& recvWithSize(cmn::sizedAlloc& msg);

   SOCKET& handle() { return m_sock; }

private:
   autoSocket& _sendWithSize(char *pPtr, int n);
   autoSocket& _recvWithSize(char *pPtr, int n);

   SOCKET m_sock;
};

class autoSockEvent {
public:
   autoSockEvent();
   ~autoSockEvent();

   autoSockEvent& observeRecv(autoSocket& sock);
   autoSockEvent& observeAccept(autoSocket& sock);
   autoSockEvent& observeConnect(autoSocket& sock);

   void waitForSecs(size_t secs, bool& timedOut);

   void waitForever(cmn::osEvent& stopSignal, bool& wasStopped);

private:
   WSAEVENT m_hEvt;
   autoSocket *m_pSock;
};

} // namespace net

#endif // ___net_wsockhlp___
