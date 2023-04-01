#include "../console/log.hpp"
#include "assert.hpp"

namespace test {

testStats::testStats()
: m_totalTests(0)
, m_totalAsserts(0)
, m_failedTests(0)
, m_currentTestFailed(false)
{
}

void testStats::addTest()
{
   m_totalTests++;
   m_currentTestFailed = false;
}

void testStats::addAssert(bool pass)
{
   m_totalAsserts++;
   if(!pass && !m_currentTestFailed)
   {
      m_failedTests++;
      m_currentTestFailed = true;
   }
}

void testStats::summarize(console::iLog& l)
{
   if(m_failedTests)
      l.writeLnInfo("result: *** FAILED ***");
   else if(m_totalTests)
      l.writeLnInfo("result: pass");
   else
      l.writeLnInfo("result: indeterminate");

   l.writeLnInfo("%d tests failed; %d total.  %d total asserts",m_failedTests,m_totalTests,m_totalAsserts);
}

asserter::asserter(console::iLog& l, testStats& s, const std::string& testName)
: m_l(l)
, m_stats(s)
, m_testName(testName)
, m_cnt(0)
{
   m_stats.addTest();
}

void asserter::assertTrue(bool value)
{
   m_cnt++;
   if(!value)
   {
      m_l.writeLnInfo("test FAILED in function '%s' on assert #%d",m_testName.c_str(),m_cnt);
      m_stats.addAssert(false);
   }
   else
      m_stats.addAssert(true);
}

void asserter::complete()
{
   if(m_cnt == 0)
   {
      m_l.writeLnInfo("tets FAILED by not asserting anything? - function '%s'",m_testName.c_str());
      m_stats.addAssert(false);
   }
   else
      m_stats.addAssert(true);
}

} // namespace test
