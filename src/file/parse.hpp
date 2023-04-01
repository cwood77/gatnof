#ifndef ___file_parser___
#define ___file_parser___

#include "api.hpp"
#include <map>
#include <string>

namespace sst {

class lexor {
public:
   enum tokens {
      kEOI,
      kLBrace,
      kRBrace,
      kLBracket,
      kRBracket,
      kColon,
      kComma,
      kQuotedStringLiteral,
      kInteger,
      kTrue,
      kFalse,
   };

   explicit lexor(const char *pPtr);

   tokens getToken();
   std::string getLexeme();

   void advance();

private:
   void buildMap();
   void eatWhitespace();
   void eatComments();
   void categorizeThumb();
   bool analyzeLexemes();
   void handleString();

   const char *m_pThumb;
   tokens m_token;
   std::string m_lexeme;
   std::map<char,tokens> m_map;
};

class parser {
public:
   parser(lexor& l, const iNodeFactory& f);

   void *parseConfig();

private:
   void parseNode(void *pNode, iNodeFactory::types ty);
   void parseDictionary(void *pNode);
   void parseArray(void *pNode);

   iNodeFactory::types determineNodeType();

   void demand(lexor::tokens t);
   void demandAndEat(lexor::tokens t);

   lexor& m_l;
   const iNodeFactory& m_f;
};

} // namespace sst

#endif // ___file_parser___
