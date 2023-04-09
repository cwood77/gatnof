#include "ir.hpp"
#include "styler.hpp"

namespace coml {

void styler::applyDefaults(objectTable& oTable)
{
   oTable.foreach<controlObject>([&](auto& o)
   {
      if(o.baseType == "cui::stringControl")
         applyDefault(o,m_ir.str);
      else if(o.baseType == "cui::intControl")
         applyDefault(o,m_ir.str);
      else if(o.baseType == "cui::buttonControl")
         applyDefault(o,m_ir.btn);
   });
}

void styler::applyDefault(controlObject& o, controlStyleDefault& d)
{
   if(o.format1.empty())
      o.format1 = d.format1;
   if(o.format2.empty())
      o.format2 = d.format2;
   if(o.format3.empty())
      o.format3 = d.format3;
   if(o.format4.empty())
      o.format4 = d.format4;
}

void styler::replaceVars(objectTable& oTable)
{
   oTable.foreach<textObject>([&](auto& o)
   {
      replaceVars(o.payload);
   });
   oTable.foreach<controlObject>([&](auto& o)
   {
      replaceVars(o.format1);
      replaceVars(o.format2);
      replaceVars(o.format3);
      replaceVars(o.format4);
   });
}

void styler::replaceVars(std::string& expr)
{
   auto it = m_ir.vars.find(expr);
   if(it != m_ir.vars.end())
      expr = it->second;
}

} // namespace coml
