#ifndef ___coml_generator___
#define ___coml_generator___

#include <ostream>
#include <string>

namespace coml {

class ir;

class streams {
public:
   streams(std::ostream& hpp, std::ostream& cpp, const std::string& hppPath)
   : m_hpp(hpp), m_cpp(cpp), m_hppPath(hppPath) {}

   std::ostream& hpp() { return m_hpp; }
   std::ostream& cpp() { return m_cpp; }
   const std::string& hppPath() const { return m_hppPath; }

private:
   std::ostream& m_hpp;
   std::ostream& m_cpp;
   std::string m_hppPath;
};

class generator {
public:
   explicit generator(ir& ir) : m_ir(ir) {}

   void generate(streams& out);

private:
   void genHeaders(streams& out);

   void genImage(streams& out);

   void genDerivedControlObjects(streams& out);

   void genScreen(streams& out);

   void generateFactory(const std::string& typeName, const std::string& cuiType, streams& out);

   ir& m_ir;
};

} // namespace coml

#endif // ___coml_generator___
