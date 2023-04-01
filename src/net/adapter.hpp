#ifndef ___net_adapter___
#define ___net_adapter___

#include "api.hpp"

namespace net {

class adapter : public iChannel {
public:
   adapter& configure(iAllocChannel& inner) { m_pInner = &inner; return *this; }

   ~adapter() { m_pInner->release(); }
   virtual void release() { delete this; }
   virtual std::string getPropStr(size_t key) { return m_pInner->getPropStr(key); }

   virtual void sendString(const std::string& msg);
   virtual std::string recvString();
   virtual std::string recvString(cmn::osEvent& stopSignal, bool& wasStopped);

   virtual void sendSst(sst::dict& d);
   virtual sst::dict *recvSst(const sst::iNodeFactory& f = sst::defNodeFactory());

   virtual void sendFile(const std::string& path);
   virtual void recvFile(const std::string& path);

private:
   iAllocChannel *m_pInner;
};

} // namespace net

#endif // ___net_adapter___
