#ifndef ___net_broadcast___
#define ___net_broadcast___

#include "../cmn/subobject.hpp"
#include "api.hpp"
#include "wsockhlp.hpp"

namespace net {

class udpSender : public iUniSender, public cmn::subobject {
public:
   virtual void release() { delete this; }

   void broadcastAndListen(const std::string& sendMessage, const std::string& expectedResponse, std::set<std::string>& responders);
};

class udpListener : public iListener, public cmn::subobject, private iResponse {
public:
   virtual void release() { delete this; }

   virtual iResponse& listen(cmn::osEvent& stopSignal, cmn::sizedAlloc& msg);

private:
   virtual void respond(void *pBytes, size_t n);
   virtual bool wasStopped() const { return m_wasStopped; }

   autoWinSockLibInit m__winsock;
   autoSocket m_sock;
   sockaddr_in m_addr;
   bool m_wasStopped;
};

} // namespace net

#endif // ___net_broadcast___
