#ifdef cdwTest
#include "../cmn/autoPtr.hpp"
#include "../cmn/win32.hpp"
#include "../console/log.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../test/api.hpp"

namespace {

class myListenerThread : public cmn::iThread {
public:
   myListenerThread(cmn::osEvent& evt) : m_evt(evt) {}

   virtual void run()
   {
      sst::dict config;
      console::nullLog log;
      tcat::typePtr<net::iNetProto> pNet;
      pNet->tie(config,log);

      cmn::autoReleasePtr<net::iAllocChannel> pAChan(&pNet->createPeerChannelServer());

      while(true)
      {
         bool wasStopped = false;
         pAChan->waitForRecv(m_evt,wasStopped);
         if(wasStopped)
            return;

         cmn::sizedAlloc msg;
         pAChan->recv(msg);

         // respond with twice as much
         cmn::sizedAlloc resp;
         resp.realloc(msg.size()*2);
         ::memcpy(resp.ptr(),msg.ptr(),msg.size());
         ::memcpy(resp.ptr()+msg.size(),msg.ptr(),msg.size());
         pAChan->send(resp);
      }
   }

private:
   cmn::osEvent& m_evt;
};

class myShortListenerThread : public cmn::iThread {
public:
   myShortListenerThread(cmn::osEvent& evt) : m_evt(evt) {}

   virtual void run()
   {
      sst::dict config;
      console::nullLog log;
      tcat::typePtr<net::iNetProto> pNet;
      pNet->tie(config,log);

      cmn::autoReleasePtr<net::iAllocChannel> pAChan(&pNet->createPeerChannelServer());

      while(true)
      {
         bool wasStopped = false;
         pAChan->waitForRecv(m_evt,wasStopped);
         if(wasStopped)
            return;

         cmn::sizedAlloc msg;
         pAChan->recv(msg);
         if(msg.size() == 4 && ::strcmp(msg.ptr(),"fee")==0)
         {
            cmn::sizedAlloc msg2;
            pAChan->recv(msg2);
            if(msg2.size() == 4 && ::strcmp(msg2.ptr(),"fie")==0)
            {
               cmn::sizedAlloc resp;
               resp.realloc(7);
               ::strcpy(resp.ptr(),"answer");
               pAChan->send(resp);
            }
            else
               throw std::runtime_error("TEST FAILED");
         }
         else
            throw std::runtime_error("TEST FAILED");
      }
   }

private:
   cmn::osEvent& m_evt;
};

} // anonymous namespace

testDefineTest(allocChannel_hammer)
{
   cmn::osEvent evt;
   myListenerThread thrd(evt);
   cmn::threadController tc(thrd);
   tc.start();

   sst::dict config;
   console::nullLog log;
   tcat::typePtr<net::iNetProto> pNet;
   pNet->tie(config,log);
   cmn::autoReleasePtr<net::iAllocChannel> pAChan(&pNet->createPeerChannelClient("localhost"));

   cmn::sizedAlloc payload;
   payload.realloc(20);
   ::strcpy(payload.ptr(),"here lies the text!");

   for(size_t i=0;i<10;i++)
   {
      auto x = payload.size();
      pAChan->send(payload);
      pAChan->recv(payload);
      a.assertTrue(payload.size() == (x*2));

      cmn::sizedAlloc expected;
      expected.realloc(payload.size());
      size_t nCopies = payload.size() / 20;
      a.assertTrue((payload.size() % 20) == 0);
      char *pThumb = expected.ptr();
      for(size_t j=0;j<nCopies;j++)
      {
         ::strcpy(pThumb,"here lies the text!");
         pThumb += 20;
      }
      a.assertTrue(::memcmp(expected.ptr(),payload.ptr(),payload.size())==0);
   }

   evt.set();
   tc.join();
}

testDefineTest(allocChannel_doubleSend)
{
   for(size_t iLoop=0;iLoop<10;iLoop++)
   {
      cmn::osEvent evt;
      myShortListenerThread thrd(evt);
      cmn::threadController tc(thrd);
      tc.start();

      sst::dict config;
      console::nullLog log;
      tcat::typePtr<net::iNetProto> pNet;
      pNet->tie(config,log);
      cmn::autoReleasePtr<net::iAllocChannel> pAChan(&pNet->createPeerChannelClient("localhost"));

      {
         cmn::sizedAlloc payload;
         payload.realloc(3+1);
         ::strcpy(payload.ptr(),"fee");
         pAChan->send(payload);
      }
      {
         cmn::sizedAlloc payload;
         payload.realloc(3+1);
         ::strcpy(payload.ptr(),"fie");
         pAChan->send(payload);
      }
      {
         cmn::sizedAlloc payload;
         pAChan->recv(payload);
         a.assertTrue(7 == payload.size());
         a.assertTrue(::strcmp(payload.ptr(),"answer")==0);
      }

      evt.set();
      tc.join();
   }
}

#endif // cdwTest
