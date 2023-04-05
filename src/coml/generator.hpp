#ifndef ___coml_generator___
#define ___coml_generator___

#include <ostream>
#include <string>

namespace coml {

class ir;

class generator {
public:
   explicit generator(ir& ir) : m_ir(ir) {}

   void generate(std::ostream& out);

private:
   void genHeaders(std::ostream& out);

   void genImage(std::ostream& out);

   void genDerivedControlObjects(std::ostream& out);

   void genScreen(std::ostream& out);

   void generateFactory(const std::string& typeName, const std::string& cuiType, std::ostream& out);

   ir& m_ir;
};

} // namespace coml

#endif // ___coml_generator___
