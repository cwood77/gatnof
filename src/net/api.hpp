#ifndef ___net_api___
#define ___net_api___

#include "../cmn/autoPtr.hpp"
#include "../cmn/subobject.hpp"
#include "../file/api.hpp"
#include <cstddef>
#include <cstring>
#include <set>
#include <string>

namespace cmn { class osEvent; }
namespace cmn { class sizedAlloc; }

namespace net {

// nimbus protocol works by broadcasting a sentinel message, and receiving replies. message
// payload is unimportant, so long as addresses of the parties are revealed

// second phase begins when broadcaster sends sync messages along P2P connections

// TODO standardize APIs here and in wsock helper

class iResponse {
public:
   virtual bool wasStopped() const = 0;

   virtual void respond(void *pBytes, size_t n) = 0;

   void respond(const std::string& msg) { respond((void*)msg.c_str(),msg.length()+1); }
};

class iListener {
public:
   virtual ~iListener() {}
   virtual void release() = 0;

   virtual iResponse& listen(cmn::osEvent& stopSignal, cmn::sizedAlloc& msg) = 0;
};

class iUniSender {
public:
   virtual ~iUniSender() {}
   virtual void release() = 0;

   virtual void broadcastAndListen(const std::string& sendMessage, const std::string& expectedResponse, std::set<std::string>& responders) = 0;
};

class iPropChannel {
public:
   enum { kConnectedIp };

   virtual ~iPropChannel() {}
   virtual void release() = 0;

   virtual std::string getPropStr(size_t key) = 0;
};

class iAllocChannel : public iPropChannel {
public:
   virtual void send(cmn::sizedAlloc& msg) = 0;
   virtual void recv(cmn::sizedAlloc& msg) = 0;
   virtual void waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped) = 0;
};

class iChannel : public iPropChannel {
public:
   virtual void sendString(const std::string& msg) = 0;
   virtual std::string recvString() = 0;
   virtual std::string recvString(cmn::osEvent& stopSignal, bool& wasStopped) = 0;

   virtual void sendSst(sst::dict& d) = 0;
   virtual sst::dict *recvSst(const sst::iNodeFactory& f = sst::defNodeFactory()) = 0;

   virtual void sendFile(const std::string& path) = 0;
   virtual void recvFile(const std::string& path) = 0;
};

class iNetProto : public virtual cmn::iSubobject {
public:
   virtual ~iNetProto() {}

   virtual iUniSender& createBroadcastSender() = 0;
   virtual iListener& createBroadcastListener() = 0;

   virtual iAllocChannel& createPeerChannelServer() = 0;
   virtual iAllocChannel& createPeerChannelClient(const std::string& ip) = 0;
   virtual iChannel& wrap(iAllocChannel& inner) = 0;
};

} // namespace net

#endif // ___net_api___
