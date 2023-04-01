#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include <windows.h>

#ifdef cdwTest
#include "../test/api.hpp"
#include "api.hpp"
#include <string.h>

testDefineTest(sst_write)
{
   sst::dict config;
   auto& installed = config.add<sst::array>("installed");
   installed.append<sst::dict>().add<sst::str>("name") = "some product";
   installed.append<sst::dict>().add<sst::str>("name") = "some other product";

   tcat::typePtr<sst::iSerializer> pWriter;
   std::string buffer = pWriter->write(config);

   const char *expected = "{\n   \"installed\": [\n      {\n         \"name\": \"some product\"\n      },\n      {\n         \"name\": \"some other product\"\n      }\n   ]\n}";
   a.assertTrue(buffer == expected);
   a.assertTrue(buffer.length() == ::strlen(expected));
}

testDefineTest(sst_default)
{
   sst::dict config;

   {
      std::string foo = config.getOpt<sst::str>("default-value","foo");
      a.assertTrue(foo == "foo");
      a.assertTrue(config.asMap().size() == 0);
   }

   {
      size_t foo = config.getOpt<sst::mint>("default-value",23);
      a.assertTrue(foo == 23);
      a.assertTrue(config.asMap().size() == 0);
   }

   {
      bool foo = config.getOpt<sst::tf>("default-value",true);
      a.assertTrue(foo == true);
      a.assertTrue(config.asMap().size() == 0);
   }

   config.add<sst::str>("default-value").set("bar");
   std::string foo = config.getOpt<sst::str>("default-value","foo");
   a.assertTrue(foo == "bar");
}

#endif // cdwTest

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
