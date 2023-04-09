#define WIN32_LEAN_AND_MEAN
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../tcatlib/api.hpp"
#include "generator.hpp"
#include "ir.hpp"
#include "mainParser.hpp"
#include "styler.hpp"
#include <fstream>
#include <memory>

namespace {

class generateCommand : public console::iCommand {
public:
   std::string oInPath;
   std::string oOutCppPath;
   std::string oStylePath;

   virtual void run(console::iLog& l);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<generateCommand>("--generate"));

      v->addParameter(
         console::stringParameter::required(offsetof(generateCommand,oInPath)));
      v->addParameter(
         console::stringParameter::required(offsetof(generateCommand,oOutCppPath)));
      v->addParameter(
         console::stringParameter::optional(offsetof(generateCommand,oStylePath)));

      return v.release();
   }
} gVerb;

void generateCommand::run(console::iLog& l)
{
   std::string outHppPath = oOutCppPath.c_str();
   {
      auto *pPtr = const_cast<char*>(outHppPath.c_str());
      pPtr[outHppPath.length() - 3] = 'h';
   }

   coml::ir ir;
   ir.computeName(oInPath);

   coml::styleIr sir;
   const bool usingStyle = !oStylePath.empty();
   if(usingStyle)
      coml::styleParser(sir).parse(oStylePath);

   coml::mainParser(ir).parse(oInPath);

   if(usingStyle)
      coml::styler(sir).apply(ir.oTable);

   {
      std::ofstream outHpp(outHppPath);
      std::ofstream outCpp(oOutCppPath);
      coml::streams out(outHpp,outCpp,outHppPath);
      coml::generator(ir).generate(out);
   }
}

} // anonymous namespace
