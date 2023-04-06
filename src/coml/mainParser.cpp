#include "../cmn/autoPtr.hpp"
#include "mainParser.hpp"
#include <cstring>
#include <fstream>
#include <sstream>

namespace coml {

void parserBase::eatUntil(const char*& pThumb, char c)
{
   for(;*pThumb!=c;++pThumb);
   ++pThumb;
}

void styleParser::parse(const std::string& inputPath)
{
   std::ifstream in(inputPath.c_str());
   while(in.good())
   {
      std::string line;
      std::getline(in,line);
      parseLine(line);
   }
}

void styleParser::parseLine(const std::string& line)
{
   if(line.empty()) return;
   if(line.c_str()[0] == ';') return;

   const char *pThumb = line.c_str();
   if(::strncmp(pThumb,"define ",7)==0)
   {
      pThumb += 7;

      cmn::zeroedBlock<> name;
      int n = 0;
      ::sscanf(pThumb,"%%%[^%]%%%n",name.b,&n);

      m_ir.vars[std::string("%") + name.b + "%"] = pThumb+n+1;
   }
   else if(::strncmp(pThumb,"default{",8)==0)
   {
      pThumb += 8;

      cmn::zeroedBlock<> type;
      cmn::zeroedBlock<> fmt1;
      cmn::zeroedBlock<> fmt2;
      cmn::zeroedBlock<> fmt3;
      cmn::zeroedBlock<> fmt4;
      ::sscanf(pThumb,"%[^}]}/%[^/]/%[^/]/%[^/]/%[^/]/",
         type.b,fmt1.b,fmt2.b,fmt3.b,fmt4.b);

      controlStyleDefault *pDft = &m_ir.str;
      if(std::string("str") == type.b)
         ;
      else if(std::string("btn") == type.b)
         pDft = &m_ir.btn;
      else
         throw std::runtime_error("style indicates default of unknown object type");
      pDft->format1 = fmt1.b;
      pDft->format2 = fmt2.b;
      pDft->format3 = fmt3.b;
      pDft->format4 = fmt4.b;
   }
   else
      throw std::runtime_error("unknown style directive");
}

void mainParser::loadLines(const std::string& inputPath)
{
   std::ifstream in(inputPath.c_str());
   while(in.good())
   {
      std::string line;
      std::getline(in,line);
      m_ir.lines.push_back(line);
   }
}

void mainParser::readHeader()
{
   // enforce ID
   if(::strncmp(m_ir.lines[0].c_str(),"coml v1 ",8) != 0)
      throw std::runtime_error("bad coml header");

   // read index
   ::sscanf(m_ir.lines[0].c_str(),"coml v1 %d@%d",&m_ir.height,&m_ir.yOffset);
}

void mainParser::buildObjectTable()
{
   for(size_t i=1+m_ir.height;;i++)
   {
      if(i >= m_ir.lines.size())
         break; // done

      bool stop = false;
      parseObjectLine(m_ir.lines[i],stop);

      if(stop)
         break; // rest are comments
   }
}

void mainParser::parseObjectLine(const std::string& line, bool& stop)
{
   if(line.empty())
      return;

   stop = (line == "; begin comments");
   if(stop)
      return;

   if(line.c_str()[0] == ';')
      return;

   int number = 0;
   ::sscanf(line.c_str(),"%d=",&number);
   const char *pThumb = line.c_str();
   eatUntil(pThumb,'=');

   auto& list = m_ir.oTable.create(number);
   parseObject(pThumb,list);
}

void mainParser::parseObject(const char*& pThumb, std::list<iObject*>& list)
{
   if(::strncmp(pThumb,"txt:",4)==0)
   {
      auto *pObj = new textObject();
      list.push_back(pObj);
      pObj->payload = pThumb + 4;
   }
   else if(::strncmp(pThumb,"str:",4)==0)
   {
      auto *pObj = new controlObject();
      list.push_back(pObj);
      pObj->baseType = "cui::stringControl";
      parseControlObject(pThumb+4,*pObj);
   }
   else if(::strncmp(pThumb,"int:",4)==0)
   {
      auto *pObj = new controlObject();
      list.push_back(pObj);
      pObj->baseType = "cui::intControl";
      parseControlObject(pThumb+4,*pObj);
   }
   else if(::strncmp(pThumb,"btn:",4)==0)
   {
      auto *pObj = new buttonControlObject();
      list.push_back(pObj);
      cmn::zeroedBlock<> name;
      cmn::zeroedBlock<> face;
      cmn::zeroedBlock<> fmt1;
      cmn::zeroedBlock<> fmt2;
      cmn::zeroedBlock<> fmt3;
      cmn::zeroedBlock<> fmt4;
      ::sscanf(pThumb,"btn:%[^/]/%d/%d/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/",
         name.b,&(pObj->length),&(pObj->height),face.b,fmt1.b,fmt2.b,fmt3.b,fmt4.b);
      pObj->name = name.b;
      pObj->face = face.b;
      pObj->format1 = fmt1.b;
      pObj->format2 = fmt2.b;
      pObj->format3 = fmt3.b;
      pObj->format4 = fmt4.b;
      pObj->baseType = "cui::buttonControl";
   }
   else if(::strncmp(pThumb,"lst:",4)==0)
   {
      auto *pObj = new listControlObject();
      list.push_back(pObj);
      cmn::zeroedBlock<> name;
      cmn::zeroedBlock<> elts;
      ::sscanf(pThumb,"lst:%[^/]/%d/%[^/]",
         name.b,&(pObj->height),elts.b);
      pObj->name = name.b;
      pObj->baseType = "cui::listControl";

      const char *pThumb = elts.b;
      while(*pThumb!=0)
      {
         int x = 0;
         int n = 0;
         ::sscanf(pThumb,"%d%n",&x,&n);
         pThumb += n;
         if(*pThumb == ',')
            pThumb++;
         pObj->elts.push_back(x);
      }
   }
   else
   {
      std::stringstream stream;
      stream << "unknown object '" << pThumb << "'";
      throw std::runtime_error(stream.str());
   }
}

void mainParser::parseControlObject(const char *pThumb, controlObject& o)
{
   cmn::zeroedBlock<> name;
   cmn::zeroedBlock<> fmt1;
   cmn::zeroedBlock<> fmt2;
   ::sscanf(pThumb,"%[^/]/%d/%d/%[^/]/%[^/]/",
      name.b,&(o.length),&(o.height),fmt1.b,fmt2.b);
   o.name = name.b;
   o.format1 = fmt1.b;
   o.format2 = fmt2.b;
}

} // namespace coml
