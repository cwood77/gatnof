#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../file/manager.hpp"
#include "../tcatlib/api.hpp"
#include "adapter.hpp"
#include <stdexcept>

namespace net {

void adapter::sendString(const std::string& msg)
{
   cmn::sizedAlloc a;
   a.realloc(msg.length()+1);
   ::strcpy(a.ptr(),msg.c_str());
   m_pInner->send(a);
}

std::string adapter::recvString()
{
   cmn::sizedAlloc a;
   m_pInner->recv(a);
   std::string v(a.ptr());
   return v;
}

std::string adapter::recvString(cmn::osEvent& stopSignal, bool& wasStopped)
{
   m_pInner->waitForRecv(stopSignal,wasStopped);
   if(wasStopped)
      return "";
   else
      return recvString();
}

void adapter::sendSst(sst::dict& d)
{
   tcat::typePtr<sst::iSerializer> pS;
   const char *pBuffer = pS->write(d);
   cmn::sizedAlloc a;
   a.realloc(::strlen(pBuffer)+1);
   ::memcpy(a.ptr(),pBuffer,a.size());
   m_pInner->send(a);
}

sst::dict *adapter::recvSst(const sst::iNodeFactory& f)
{
   cmn::sizedAlloc a;
   m_pInner->recv(a);

   tcat::typePtr<sst::iDeserializer> pD;
   return pD->parse(a.ptr(),f);
}

void adapter::sendFile(const std::string& path)
{
   cmn::sizedAlloc a;
   {
      cmn::autoCFilePtr f(path,"rb");
      long x = f.calculateFileSizeFromStart();
      a.realloc(x);
      ::fread(a.ptr(),1,a.size(),f.fp);
   }
   m_pInner->send(a);
}

void adapter::recvFile(const std::string& path)
{
   cmn::sizedAlloc a;
   m_pInner->recv(a);
   {
      tcat::typePtr<file::iFileManager> fMan;
      console::nullLog nowhere;
      fMan->createAllFoldersForFile(path.c_str(),nowhere,/*really*/true);
      cmn::autoCFilePtr f(path,"wb");
      ::fwrite(a.ptr(),1,a.size(),f.fp);
   }
}

} // namespace net
