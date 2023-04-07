#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include <windows.h>

#ifdef cdwTest
#include "../test/api.hpp"
#include "api.hpp"

testDefineTest(cui_maxValueIntFormatter)
{
   cui::maxValueIntFormatter sut;
   a.assertTrue(sut.formatValue(1,3) == "1");
   a.assertTrue(sut.formatValue(12,3) == "12");
   a.assertTrue(sut.formatValue(99,3) == "99");
   a.assertTrue(sut.formatValue(100,3) == "99+");
}

testDefineTest(cui_hugeValueIntFormatter)
{
   cui::hugeValueIntFormatter sut;
   a.assertTrue(sut.formatValue(2147483647L,4) == "2.1B");
   a.assertTrue(sut.formatValue( 214748364L,4) == "214M");
   a.assertTrue(sut.formatValue(  21474836L,4) ==  "21M");
   a.assertTrue(sut.formatValue(   2147483L,4) == "2.1M");
   a.assertTrue(sut.formatValue(    214748L,4) == "214k");
   a.assertTrue(sut.formatValue(     21474L,4) ==  "21k");
   a.assertTrue(sut.formatValue(      2147L,4) == "2147");
   a.assertTrue(sut.formatValue(       214L,4) ==  "214");
   a.assertTrue(sut.formatValue(        21L,4) ==   "21");
   a.assertTrue(sut.formatValue(         2L,4) ==    "2");

   a.assertTrue(sut.formatValue(LONG_MAX,4)  == "2.1B");
}

#endif // cdwTest

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
