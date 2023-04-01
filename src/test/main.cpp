#define WIN32_LEAN_AND_MEAN
#include "../cmn/autoPtr.hpp"
#include "../console/log.hpp"
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include "assert.hpp"
#include <exception>
#include <stddef.h>
#include <windows.h>

int main(int argc, const char *argv[])
{
   console::cStdOutLogSink logSink;
   tcat::typePtr<console::iLog> pLog;
   pLog->sink(&logSink);
   pLog->configureForBackground();

   pLog->writeLnInfo("looking for tests");
   test::testStats stats;

   try
   {
      tcat::typeSet<test::iTest> tests;
      pLog->writeLnInfo("found %lld tests",tests.size());
      for(size_t i=0;i<tests.size();i++)
      {
         test::iTest *pTest = tests[i];
         pLog->writeLnInfo("   running %s",pTest->getName());
         test::asserter a(*pLog,stats,pTest->getName());
         pTest->run(a);
         a.complete();
      }
      stats.summarize(*pLog);
   }
   catch(std::exception& x)
   {
      pLog->writeLnInfo("ERROR: %s",x.what());
   }

   return 0;
}
