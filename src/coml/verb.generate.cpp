#define WIN32_LEAN_AND_MEAN
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../tcatlib/api.hpp"
#include "generator.hpp"
#include "ir.hpp"
#include "mainParser.hpp"
#include <fstream>
#include <memory>

namespace {

class generateCommand : public console::iCommand {
public:
   std::string oInPath;
   std::string oOutPath;

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
         console::stringParameter::required(offsetof(generateCommand,oOutPath)));

      return v.release();
   }
} gVerb;

void generateCommand::run(console::iLog& l)
{
   coml::ir ir;
   ir.computeName(oInPath);

   coml::mainParser(ir).parse(oInPath);

   std::ofstream out(oOutPath);
   coml::generator(ir).generate(out);
}

} // anonymous namespace
