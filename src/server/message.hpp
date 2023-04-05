#ifndef ___server_msg___
#define ___server_msg___

#include "../cmn/subobject.hpp"
#include <map>
#include <string>

namespace file { class iSstFile; }
namespace net { class iChannel; }

namespace server {

class connectionContext {
public:
   connectionContext() : quit(false), pAcct(NULL) {}

   bool quit;
   file::iSstFile *pAcct;
};

class iMsgHandler : public cmn::subobject {
public:
   virtual void run(net::iChannel& ch, connectionContext& ctxt) = 0;
};

class msgRegistry {
public:
   static msgRegistry& get();

   void publish(const std::string& cmd, iMsgHandler& h) { m_msgs[cmd] = &h; }

   iMsgHandler& demand(const std::string& cmd) { return *m_msgs[cmd]; }

private:
   std::map<std::string,iMsgHandler*> m_msgs;
};

template<class T>
class msgRegistrar {
public:
   explicit msgRegistrar(const std::string& cmd, msgRegistry& r = msgRegistry::get())
   {
      r.publish(cmd,m_mh);
   }

private:
   T m_mh;
};

} // namespace server

#endif // ___server_msg___
