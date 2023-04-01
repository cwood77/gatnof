#ifdef cdwTest
#include "../cmn/autoPtr.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../test/api.hpp"
#include <list>

namespace {

class myFakeAllocChannel : public net::iAllocChannel {
public:
   myFakeAllocChannel() : wasDeleted(false) {}

   virtual void release() { wasDeleted = true; }

   virtual std::string getPropStr(size_t key)
   {
      throw std::runtime_error("unimpled");
   }

   virtual void send(cmn::sizedAlloc& msg)
   {
      m_allocs.push_back(cmn::sizedAlloc());
      auto& a = m_allocs.back();
      a.realloc(msg.size());
      ::memcpy(a.ptr(),msg.ptr(),msg.size());
   }

   virtual void recv(cmn::sizedAlloc& msg)
   {
      if(m_allocs.size() == 0)
         throw std::runtime_error("reading from an empty fake channel");

      auto& a = m_allocs.front();
      msg.realloc(a.size());
      ::memcpy(msg.ptr(),a.ptr(),a.size());
      m_allocs.pop_front();
   }

   virtual void waitForRecv(cmn::osEvent& stopSignal, bool& wasStopped)
   {
      throw std::runtime_error("unimpled");
   }

   bool wasDeleted;
   std::list<cmn::sizedAlloc> m_allocs;
};

} // anonymous namespace

testDefineTest(iChannel_string_roundtrip)
{
   sst::dict config;
   console::nullLog nowhere;

   tcat::typePtr<net::iNetProto> pNetProt;
   pNetProt->tie(config,nowhere);

   myFakeAllocChannel fake;

   {
      cmn::autoReleasePtr<net::iChannel> pChan(&pNetProt->wrap(fake));

      for(size_t i=0;i<20;i++)
      {
         pChan->sendString("here lies the text");
         a.assertTrue("here lies the text" == pChan->recvString());
      }
   }

   a.assertTrue(fake.wasDeleted);
   a.assertTrue(0 == fake.m_allocs.size());
}

testDefineTest(iChannel_sst_roundtrip)
{
   sst::dict config;
   console::nullLog nowhere;

   tcat::typePtr<net::iNetProto> pNetProt;
   pNetProt->tie(config,nowhere);

   myFakeAllocChannel fake;

   {
      cmn::autoReleasePtr<net::iChannel> pChan(&pNetProt->wrap(fake));

      sst::dict dict1;
      dict1.add<sst::str>("name") = "foo";
      dict1.add<sst::array>("a").append<sst::str>() = "bar";
      pChan->sendSst(dict1);

      std::unique_ptr<sst::dict> pDict2(pChan->recvSst());
      a.assertTrue((*pDict2)["name"].as<sst::str>().get() == "foo");
      a.assertTrue((*pDict2)["a"].as<sst::array>().size() == 1);
   }

   a.assertTrue(fake.wasDeleted);
   a.assertTrue(0 == fake.m_allocs.size());
}

#endif // cdwTest
