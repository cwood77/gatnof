#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include <windows.h>

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
   myListenerThread(
      cmn::osEvent& evt,
      const std::string& expected,
      const std::string& matchAns,
      const std::string& noMatchAns)
   : m_evt(evt), m_expected(expected), m_matchAns(matchAns), m_noMatchAns(noMatchAns) {}

   virtual void run()
   {
      while(true)
      {
         sst::dict config;
         console::nullLog log;
         tcat::typePtr<net::iNetProto> pNet;
         pNet->tie(config,log);

         cmn::autoReleasePtr<net::iListener> pList(&pNet->createBroadcastListener());
         cmn::sizedAlloc a;
         a.realloc(1024);
         auto& r = pList->listen(m_evt,a);
         if(r.wasStopped())
            return;

         if(m_expected == a.ptr())
            r.respond(m_matchAns);
         else
            r.respond(m_noMatchAns);
      }
   }

private:
   cmn::osEvent& m_evt;
   std::string m_expected;
   std::string m_matchAns;
   std::string m_noMatchAns;
};

} // anonymous namespace

void broadcast_endToEnd_helper(test::iAsserter& a, bool respond)
{
   cmn::osEvent evt;
   myListenerThread thrd(evt,respond ? "server-says-hi" : "nope","hi-back","what?");
   cmn::threadController tc(thrd);
   tc.start();

   sst::dict config;
   config.add<sst::mint>("network-timeout-in-seconds") = 1;
   console::nullLog log;
   tcat::typePtr<net::iNetProto> pNet;
   pNet->tie(config,log);
   cmn::autoReleasePtr<net::iUniSender> pSvr(&pNet->createBroadcastSender());
   std::set<std::string> ans;
   pSvr->broadcastAndListen("server-says-hi","hi-back",ans);

   evt.set();
   tc.join();

   a.assertTrue(respond ? ans.size()==1 : ans.size() == 0);
}

testDefineTest(broadcast_endToEnd_respond)
{
   broadcast_endToEnd_helper(a,/*respond*/true);
}

testDefineTest(broadcast_endToEnd_noRespond)
{
   broadcast_endToEnd_helper(a,/*respond*/false);
}

#endif // cdwTest

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
