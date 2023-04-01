#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include "adapter.hpp"
#include "api.hpp"
#include "broadcast.hpp"
#include "channel.hpp"
#include <stdexcept>
#include <windows.h>

namespace net {

class netProto : public iNetProto, public cmn::subobject {
public:
   virtual iUniSender& createBroadcastSender() { return tie<udpSender>(); }
   virtual iListener& createBroadcastListener() { return tie<udpListener>(); }

   virtual iAllocChannel& createPeerChannelServer() { return tie<peerChannelServer>(); }
   virtual iAllocChannel& createPeerChannelClient(const std::string& ip)
   { return tie<peerChannelClient>().open(ip); }

   virtual iChannel& wrap(iAllocChannel& inner) { return (new adapter)->configure(inner); }
};

tcatExposeTypeAs(netProto,iNetProto);

} // namespace net

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
