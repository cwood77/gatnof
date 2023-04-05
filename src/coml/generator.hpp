#ifndef ___coml_generator___
#define ___coml_generator___

#include <string>
#include <ostream>

namespace coml {

class ir;

class generator {
public:
   void generate(const std::string& name, const std::string& outPath, ir& ir);

private:
   void generateFactory(const std::string& typeName, const std::string& cuiType, std::ostream& out);
};

} // namespace coml

#endif // ___coml_generator___
