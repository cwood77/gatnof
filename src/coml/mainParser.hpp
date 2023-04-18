#ifndef ___coml_mainParser___
#define ___coml_mainParser___

#include "ir.hpp"
#include <list>
#include <string>
#include <vector>

namespace coml {

class iObject;
class objectTable;

class parserBase {
protected:
   static void eatUntil(const char*& pThumb, char c);
};

class styleParser : public parserBase {
public:
   explicit styleParser(styleIr& ir) : m_ir(ir) {}

   void parse(const std::string& inputPath);

private:
   void parseLine(const std::string& line);

   styleIr& m_ir;
};

class mainParser : public parserBase {
public:
   explicit mainParser(ir& ir) : m_ir(ir) {}

   void parse(const std::string& inputPath)
   {
      loadLines(inputPath);
      readHeader();
      buildObjectTable();
   }

private:
   void loadLines(const std::string& inputPath);
   void readHeader();

   void buildObjectTable();
   void parseObjectLine(const std::string& line, bool& stop);
   void parseObject(const char*& pThumb, std::list<iObject*>& list);
   void parseControlObject(const char *pThumb, controlObject& o);
   void cleanUpFormats(controlObject& o);

   ir& m_ir;
};

} // namespace coml

#endif // ___coml_mainParser___
