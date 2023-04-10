#include "generator.hpp"
#include "ir.hpp"

namespace coml {

void generator::generate(streams& out)
{
   genHeaders(out);

   genImage(out);

   genDerivedControlObjects(out);

   genScreen(out);
}

void generator::genHeaders(streams& out)
{
   out.hpp() << "// *******************************" << std::endl;
   out.hpp() << "// ** code generated by coml" << std::endl;
   out.hpp() << "// *******************************" << std::endl;
   out.hpp() << std::endl;
   out.hpp() << "#include \"../../src/cmn/service.hpp\"" << std::endl;
   out.hpp() << "#include \"../../src/cui/api.hpp\"" << std::endl;
   out.hpp() << "#include \"../../src/cui/pen.hpp\"" << std::endl;
   out.hpp() << "#include \"../../src/tcatlib/api.hpp\"" << std::endl;
   out.hpp() << "#include <iostream>" << std::endl;

   out.cpp() << "// *******************************" << std::endl;
   out.cpp() << "// ** code generated by coml" << std::endl;
   out.cpp() << "// *******************************" << std::endl;
   out.cpp() << std::endl;
   out.cpp() << "#include \"../../" << out.hppPath() << "\"" << std::endl;
}

void generator::genImage(streams& out)
{
   out.hpp() << std::endl;
   out.hpp() << "class " << m_ir.name << "_image : public cui::iImage {" << std::endl;
   out.hpp() << "public:" << std::endl;
   out.hpp() << "   virtual void render()" << std::endl;
   out.hpp() << "   {" << std::endl;
   out.hpp() << "      tcat::typePtr<cmn::serviceManager> svcMan;" << std::endl;
   out.hpp() << "      auto& pn = svcMan->demand<pen::object>();" << std::endl;
   out.hpp() << std::endl;

   if(m_ir.yOffset != 1)
      out.hpp() << "      pn.str() << pen::moveTo(cui::pnt(1," << m_ir.yOffset << "));" << std::endl;

   for(int i=0;i<m_ir.height;i++)
   {
      out.hpp() << "      pn.str() << \"";

      size_t jOffset = 0;
      auto& line = m_ir.lines[i+1];
      for(size_t j=0;j!=line.length();j++)
      {
         if(::isdigit(line.c_str()[j]))
         {
            int id = 0;
            int n = 0;
            ::sscanf(line.c_str()+j,"%d%n",&id,&n);
            auto& objs = m_ir.oTable.demand(id);
            for(auto *pObj : objs)
            {
               pObj->place(cui::pnt(j+1-jOffset,i+m_ir.yOffset));
               pObj->render(out.hpp());
            }
            jOffset += n;
            j += (n-1);
         }
         else if(line.c_str()[j] == '_')
            jOffset++; // ignore this char
         else
            out.hpp() << std::string(1,line.c_str()[j]);
      }
      out.hpp() << "\" << std::endl;" << std::endl;
   }

   out.hpp() << "   }" << std::endl;
   out.hpp() << std::endl;
   out.hpp() << "   virtual cui::pnt demandPnt(const std::string& id) { throw 3.14; }" << std::endl;
   out.hpp() << "};" << std::endl;

   // generate the image factory
   generateFactory(m_ir.name + "_image","cui::iImage",out);
}

void generator::genDerivedControlObjects(streams& out)
{
   m_ir.oTable.foreach<controlObject>([&](auto& ctl)
   {
      if(dynamic_cast<listControlObject*>(&ctl))
         // lists do special generation in a second
         return;

      out.hpp() << std::endl;
      out.hpp() << "class " << m_ir.name << "_" << ctl.name << "_ctl : public " << ctl.baseType << " {" << std::endl;
      out.hpp() << "protected:" << std::endl;
      out.hpp() << "   virtual void formatText1(std::ostream& o)" << std::endl;
      out.hpp() << "   {" << std::endl;
      if(!ctl.format1.empty())
         out.hpp() << "      o" << ctl.format1 << ";" << std::endl;
      out.hpp() << "   }" << std::endl;
      out.hpp() << "   virtual void formatText2(std::ostream& o)" << std::endl;
      out.hpp() << "   {" << std::endl;
      if(!ctl.format2.empty())
         out.hpp() << "      o" << ctl.format2 << ";" << std::endl;
      out.hpp() << "   }" << std::endl;
      out.hpp() << "   virtual void formatText3(std::ostream& o)" << std::endl;
      out.hpp() << "   {" << std::endl;
      if(!ctl.format3.empty())
         out.hpp() << "      o" << ctl.format3 << ";" << std::endl;
      out.hpp() << "   }" << std::endl;
      out.hpp() << "   virtual void formatText4(std::ostream& o)" << std::endl;
      out.hpp() << "   {" << std::endl;
      if(!ctl.format4.empty())
         out.hpp() << "      o" << ctl.format4 << ";" << std::endl;
      out.hpp() << "   }" << std::endl;
      out.hpp() << "};" << std::endl;
   });

   m_ir.oTable.foreach<listControlObject>([&](auto& ctl)
   {
      out.hpp() << std::endl;
      out.hpp() << "class " << m_ir.name << "_" << ctl.name << "_row_ctl {" << std::endl;
      out.hpp() << "public:" << std::endl;
      for(size_t eltId : ctl.elts)
      {
         auto& oList = m_ir.oTable.demand(eltId);
         auto *pCtl = dynamic_cast<controlObject*>(*oList.begin());
         pCtl->isTableElt = true;

         out.hpp() << "   " << m_ir.name << "_" << pCtl->name << "_ctl " << pCtl->name << ";" << std::endl;
      }
      out.hpp() << std::endl;
      out.hpp() << "   void initialize(size_t yOffset)" << std::endl;
      out.hpp() << "   {" << std::endl;
      for(size_t eltId : ctl.elts)
      {
         auto& oList = m_ir.oTable.demand(eltId);
         auto *pCtl = dynamic_cast<controlObject*>(*oList.begin());

         out.hpp() << "      " << pCtl->name << ".initialize(cui::pnt(" << pCtl->pnt.x << "," << pCtl->pnt.y << "+yOffset)," << pCtl->length << "," << pCtl->height << ");" << std::endl;
      }
      out.hpp() << "   }" << std::endl;
      out.hpp() << std::endl;
      out.hpp() << "   void erase()" << std::endl;
      out.hpp() << "   {" << std::endl;
      for(size_t eltId : ctl.elts)
      {
         auto& oList = m_ir.oTable.demand(eltId);
         auto *pCtl = dynamic_cast<controlObject*>(*oList.begin());

         out.hpp() << "      " << pCtl->name << ".erase();" << std::endl;
      }
      out.hpp() << "   }" << std::endl;
      out.hpp() << "};" << std::endl;
   });
}

void generator::genScreen(streams& out)
{
   out.hpp() << std::endl;
   out.hpp() << "class " << m_ir.name << "_screen : public cui::basicScreen {" << std::endl;
   out.hpp() << "protected:" << std::endl;
   out.hpp() << "   " << m_ir.name << "_image m_image;" << std::endl;
   m_ir.oTable.foreach<controlObject>([&](auto& ctl)
   {
      if(ctl.isTableElt) return;
      if(dynamic_cast<listControlObject*>(&ctl))
         out.hpp() << "   cui::listControl<" << m_ir.name << "_" << ctl.name << "_row_ctl> m_"
            << ctl.name << ";" << std::endl;
      else
         out.hpp() << "   " << m_ir.name << "_" << ctl.name << "_ctl m_"
            << ctl.name << ";" << std::endl;
   });
   out.hpp() << std::endl;
   out.hpp() << "public:" << std::endl;
   out.hpp() << "   " << m_ir.name << "_screen()" << std::endl;
   out.hpp() << "   {" << std::endl;
   out.hpp() << "      publishObject(\"\",m_image);" << std::endl;
   m_ir.oTable.foreach<controlObject>([&](auto& ctl)
   {
      if(ctl.isTableElt) return;
      out.hpp() << "      publishObject(\"" << ctl.name << "\",m_" << ctl.name << ");" << std::endl;
   });
   m_ir.oTable.foreach<buttonControlObject>([&](auto& ctl)
   {
      out.hpp() << "      m_" << ctl.name << ".setFaceText(\"" << ctl.face << "\");" << std::endl;
   });
   out.hpp() << "   }" << std::endl;
   out.hpp() << std::endl;
   out.hpp() << "   virtual void render()" << std::endl;
   out.hpp() << "   {" << std::endl;
   out.hpp() << "      m_image.render();" << std::endl;
   m_ir.oTable.foreach<controlObject>([&](auto& ctl)
   {
      if(ctl.isTableElt) return;
      out.hpp() << "      m_" << ctl.name << ".initialize(cui::pnt(" << ctl.pnt.x << "," << ctl.pnt.y << ")," << ctl.length << "," << ctl.height << ");" << std::endl;
   });
   out.hpp() << "   }" << std::endl;
   out.hpp() << "};" << std::endl;

   generateFactory(m_ir.name + "_screen","cui::iScreen",out);
}

void generator::generateFactory(const std::string& typeName, const std::string& cuiType, streams& out)
{
   // generate the image factory
   out.cpp() << std::endl;
   out.cpp() << "class " << typeName << "_fac : public cui::plugInFactoryT<" << typeName << "," << cuiType << "> {" << std::endl;
   out.cpp() << "public:" << std::endl;
   out.cpp() << "   " << typeName << "_fac() : cui::plugInFactoryT<" << typeName << "," << cuiType << ">(\"" << typeName << "\") {}" << std::endl;
   out.cpp() << "};" << std::endl;
   out.cpp() << std::endl;
   out.cpp() << "tcatExposeTypeAs(" << typeName << "_fac,cui::iPlugInFactory);" << std::endl;
}

} // namespace coml
