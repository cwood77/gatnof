#ifndef ___net_channel___
#define ___net_channel___

#include "../cmn/subobject.hpp"
#include "api.hpp"
#include "wsockhlp.hpp"

namespace net {

class peerChannelBase : public iAllocChannel, public cmn::subobject {
public:
   virtual void release() { delete this; }
   virtual std::string getPropStr(size_t key);
   virtual void send(cmn::sizedAlloc& msg);
   virtual void recv(cmn::sizedAlloc& msg);
   virtual void waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped);

protected:
   autoWinSockLibInit m__winsock;
   autoSocket m_sock;
};

class peerChannelServer : public peerChannelBase {
public:
   peerChannelServer() : m_isOpened(false) {}

   virtual std::string getPropStr(size_t key);
   virtual void send(cmn::sizedAlloc& msg);
   virtual void recv(cmn::sizedAlloc& msg);
   virtual void waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped);

private:
   void open(cmn::osEvent& stopSignal, bool& wasStopped);

   bool m_isOpened;
   std::string m_connectedIp;
};

class peerChannelClient : public peerChannelBase {
public:
   peerChannelClient& open(const std::string& ip);
};

} // namespace net

#endif // ___net_channel___
