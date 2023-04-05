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

class mainParser : public parserBase {
public:
   void loadAndVerify(const std::string& path, std::vector<std::string>& lines, int& height);

   void loadAndVerify(const std::string& path, ir& i)
   { loadAndVerify(path,i.lines,i.height); }

   void parseObjectLine(const std::string& line, bool& stop, objectTable& oTable);

private:
   void parseObject(const char*& pThumb, std::list<iObject*>& list);
};

} // namespace coml

#endif // ___coml_mainParser___
