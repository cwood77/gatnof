#ifndef ___test_assert___
#define ___test_assert___

#include "api.hpp"

namespace console { class iLog; }
namespace test {

class testStats {
public:
   testStats();
   void addTest();
   void addAssert(bool pass);
   void summarize(console::iLog& l);

private:
   size_t m_totalTests;
   size_t m_totalAsserts;
   size_t m_failedTests;
   bool m_currentTestFailed;
};

class asserter : public iAsserter {
public:
   asserter(console::iLog& l, testStats& s, const std::string& testName);
   
   virtual void assertTrue(bool value);

   void complete();

private:
   console::iLog& m_l;
   testStats& m_stats;
   std::string m_testName;
   int m_cnt;
};

} // namespace test

#endif // ___test_assert___
