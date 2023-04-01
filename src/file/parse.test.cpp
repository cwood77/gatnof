#ifdef cdwTest
#include "../test/api.hpp"
#include "parse.hpp"
#include <exception>

using namespace sst;

testDefineTest(file_lexor_general)
{
   const char *text = " {,\t\r\n} \"bleh \r \t\":[ # foo } \r\n ] ";

   lexor l(text);
   a.assertTrue(l.getToken() == lexor::kLBrace);
   a.assertTrue(l.getToken() == lexor::kLBrace);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kComma);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kRBrace);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kQuotedStringLiteral);
   a.assertTrue(l.getLexeme() == "bleh \r \t");
   l.advance();
   a.assertTrue(l.getToken() == lexor::kColon);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kLBracket);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kRBracket);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kEOI);
}

testDefineTest(file_lexor_unterminatedStrLit)
{
   const char *text = "  [ \"foo   ";

   lexor l(text);
   a.assertTrue(l.getToken() == lexor::kLBracket);
   try
   {
      l.advance();
      a.assertTrue(false);
   }
   catch(std::exception&)
   {
      a.assertTrue(true);
   }
}

testDefineTest(file_lexor_consecutiveComments)
{
   const char *text = "  [  #\r\n#\n# bleh\n}";

   lexor l(text);
   a.assertTrue(l.getToken() == lexor::kLBracket);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kRBrace);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kEOI);
}

testDefineTest(file_lexor_mintAndBool)
{
   const char *text = "true 12 false 0";

   lexor l(text);
   a.assertTrue(l.getToken() == lexor::kTrue);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kInteger);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kFalse);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kInteger);
   l.advance();
   a.assertTrue(l.getToken() == lexor::kEOI);
}

testDefineTest(file_parser_general)
{
   const char *text = "{ \"foo\": \"bar\" }";
   defNodeFactory f;

   lexor l(text);
   parser p(l,f);

   node& c = *(node*)p.parseConfig();

   a.assertTrue(c.as<dict>()["foo"].as<str>().get() == "bar");
}

testDefineTest(file_parser_nesting)
{
   const char *text = "{ \"foo\": \"bar\", \"baz\": [ 7, true ] }";
   defNodeFactory f;

   lexor l(text);
   parser p(l,f);

   node& c = *(node*)p.parseConfig();

   a.assertTrue(c.as<dict>()["foo"].as<str>().get() == "bar");
   a.assertTrue(c.as<dict>()["baz"].as<array>().size() == 2);
   a.assertTrue(c.as<dict>()["baz"].as<array>()[0].as<mint>().get() == 7);
   a.assertTrue(c.as<dict>()["baz"].as<array>()[1].as<tf>().get());
}

#endif // cdwTest
