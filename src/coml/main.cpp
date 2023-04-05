#include "generator.hpp"
#include "ir.hpp"
#include "mainParser.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace coml;

int main(int argc, const char *argv[])
{
   std::cout << "{{" << std::endl;
   for(int i=0;i<argc;i++)
   {
      std::cout << "  " << argv[i] << std::endl;
   }
   std::cout << "}}" << std::endl;

   // compute name
   std::string name;
   {
      const char *pThumb = ::strstr(argv[1],"/screen.") + 8;
      if(!pThumb)
      {
         std::cerr << "bad coml file path" << std::endl;
         return -1;
      }
      name = std::string(pThumb,::strlen(pThumb) - 5);
   }

   coml::mainParser parser;
   coml::ir ir;
   parser.loadAndVerify(argv[1],ir);

   // build object table
   objectTable& oTable = ir.oTable; // ALIAS
   for(size_t i=1+ir.height;;i++)
   {
      if(i >= ir.lines.size())
         break; // done
      bool stop = false;
      parser.parseObjectLine(ir.lines[i],stop,oTable);
      if(stop)
         break; // rest are comments
   }

   generator g;
   g.generate(name,argv[2],ir);

   return 0;
}
